#include "mqtt.h"
#include "settings.h"
#include "feeder.h"
#include "webserver.h"          // fmtWhen
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

static WiFiClient        netPlain;
static WiFiClientSecure  netTls;
static PubSubClient      mqtt;
static char  topCmd[80], topStat[80], topOnline[80], topEvt[80];
static bool  conectado  = false;
static uint32_t proxPub = 0;

bool mqttEnabled() { return strlen(cfg.mqttHost) > 3; }
bool mqttOnline()  { return conectado; }

// ------------------------------------------------------------------ topicos
static void montaTopicos() {
    const char *p = strlen(cfg.mqttPrefix) ? cfg.mqttPrefix : "aquafeeder";
    snprintf(topCmd,    sizeof(topCmd),    "%s/cmd",    p);
    snprintf(topStat,   sizeof(topStat),   "%s/status", p);
    snprintf(topOnline, sizeof(topOnline), "%s/online", p);
    snprintf(topEvt,    sizeof(topEvt),    "%s/evento", p);
}

// ------------------------------------------------------------------- status
static void publicaStatus() {
    if (!conectado) return;
    JsonDocument d;
    char w[24];
    fmtWhen(feederLastFeed(), w, sizeof(w));
    d["porcoes_hoje"] = feederFedToday();
    d["limite_dia"]   = cfg.maxPerDay;
    d["ultima"]       = w;
    d["ocupado"]      = feederBusy();
    d["total"]        = feederTotalPortions();
    d["rssi"]         = WiFi.RSSI();
    d["ip"]           = WiFi.localIP().toString();
    d["uptime_s"]     = (uint32_t)(millis() / 1000);
    d["heap"]         = ESP.getFreeHeap();
    d["fw"]           = FW_VERSION;
    char buf[420];
    size_t n = serializeJson(d, buf, sizeof(buf));
    mqtt.publish(topStat, (const uint8_t *)buf, n, true);   // retido
}

void mqttNotifyFeed(uint8_t portions, const char *src, bool ok, uint16_t pulses) {
    if (!conectado) return;
    JsonDocument d;
    d["porcoes"] = portions;
    d["origem"]  = src;
    d["ok"]      = ok;
    if (cfg.sensorEnabled) d["graos"] = pulses;
    char buf[160];
    size_t n = serializeJson(d, buf, sizeof(buf));
    mqtt.publish(topEvt, (const uint8_t *)buf, n, false);
    publicaStatus();
}

// ------------------------------------------------------------------ comandos
// Aceita JSON  {"cmd":"feed","p":2}  ou texto puro  "feed 2"
static void executa(const String &cmd, int arg) {
    String err;
    if (cmd == "feed" || cmd == "alimentar") {
        uint8_t p = (arg > 0) ? (uint8_t)arg : 1;
        if (feederRequest(p, SRC_TELEGRAM, err))
            Serial.printf("[mqtt] alimentando %u porcao(oes)\n", p);
        else {
            Serial.printf("[mqtt] barrado: %s\n", err.c_str());
            char buf[160];
            snprintf(buf, sizeof(buf), "{\"erro\":\"%s\"}", err.c_str());
            mqtt.publish(topEvt, buf);
        }
    } else if (cmd == "jog") {
        feederJog(arg ? arg : STEPS_PER_REV / 4);
    } else if (cmd == "status") {
        publicaStatus();
    } else if (cmd == "reboot" || cmd == "reiniciar") {
        Serial.println(F("[mqtt] reiniciando por comando"));
        mqtt.publish(topOnline, "0", true);
        delay(300);
        ESP.restart();
    } else {
        Serial.printf("[mqtt] comando desconhecido: %s\n", cmd.c_str());
    }
}

static void aoReceber(char *topic, uint8_t *payload, unsigned int len) {
    String msg;
    msg.reserve(len + 1);
    for (unsigned int i = 0; i < len; i++) msg += (char)payload[i];
    msg.trim();
    Serial.printf("[mqtt] %s <- %s\n", topic, msg.c_str());

    if (msg.startsWith("{")) {
        JsonDocument d;
        if (deserializeJson(d, msg)) return;
        String c = d["cmd"] | "";
        c.toLowerCase();
        executa(c, d["p"] | d["steps"] | 0);
    } else {
        int sp = msg.indexOf(' ');
        String c = (sp > 0) ? msg.substring(0, sp) : msg;
        int a = (sp > 0) ? msg.substring(sp + 1).toInt() : 0;
        c.toLowerCase();
        executa(c, a);
    }
}

// -------------------------------------------------------------------- conexao
static bool conecta() {
    if (WiFi.status() != WL_CONNECTED) return false;

    if (cfg.mqttTls) {
        netTls.setInsecure();          // ver nota de seguranca na doc
        mqtt.setClient(netTls);
    } else {
        mqtt.setClient(netPlain);
    }
    mqtt.setServer(cfg.mqttHost, cfg.mqttPort ? cfg.mqttPort : (cfg.mqttTls ? 8883 : 1883));
    mqtt.setCallback(aoReceber);
    mqtt.setBufferSize(768);
    mqtt.setKeepAlive(30);
    mqtt.setSocketTimeout(15);

    char id[40];
    snprintf(id, sizeof(id), "%s-%04X", cfg.host, (uint16_t)(ESP.getEfuseMac() & 0xFFFF));

    // Last Will: se a conexao cair, o BROKER publica "0" em <prefixo>/online
    bool ok = mqtt.connect(id,
                           strlen(cfg.mqttUser) ? cfg.mqttUser : nullptr,
                           strlen(cfg.mqttPass) ? cfg.mqttPass : nullptr,
                           topOnline, 0, true, "0", true);
    if (!ok) {
        Serial.printf("[mqtt] falha ao conectar (state=%d)\n", mqtt.state());
        return false;
    }
    conectado = true;
    mqtt.publish(topOnline, "1", true);
    mqtt.subscribe(topCmd, 1);
    Serial.printf("[mqtt] conectado em %s, assinando %s\n", cfg.mqttHost, topCmd);
    publicaStatus();
    return true;
}

static void tarefa(void *) {
    vTaskDelay(pdMS_TO_TICKS(4000));
    uint32_t proxTentativa = 0;
    for (;;) {
        if (!mqttEnabled()) { vTaskDelay(pdMS_TO_TICKS(5000)); continue; }

        if (!mqtt.connected()) {
            conectado = false;
            if (millis() >= proxTentativa) {
                if (!conecta()) proxTentativa = millis() + 10000;
            }
            vTaskDelay(pdMS_TO_TICKS(500));
            continue;
        }
        mqtt.loop();
        if (millis() >= proxPub) {
            proxPub = millis() + 60000;
            publicaStatus();
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void mqttBegin() {
    if (!mqttEnabled()) { Serial.println(F("[mqtt] desligado (sem broker)")); return; }
    montaTopicos();
    xTaskCreatePinnedToCore(tarefa, "mqtt", 8192, nullptr, 1, nullptr, 0);
    Serial.printf("[mqtt] task iniciada -> %s:%u%s\n",
                  cfg.mqttHost, cfg.mqttPort, cfg.mqttTls ? " (TLS)" : "");
}
