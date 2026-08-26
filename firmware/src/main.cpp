// =====================================================================
//  AquaFeeder ESP32-S3
//  Alimentador de aquario com agenda, acionamento manual e camera.
//
//  Primeiro boot: nao ha Wi-Fi salvo -> sobe o modo AP
//    rede "AquaFeeder-xxxx", senha "aquafeeder", abra http://192.168.4.1
//    em Ajustes > Rede coloque seu Wi-Fi e salve (o aparelho reinicia).
// =====================================================================
#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <ArduinoOTA.h>

#include "config.h"
#include "settings.h"
#include "feeder.h"
#include "camera.h"
#include "webserver.h"
#include "telegram.h"
#include "mqtt.h"
#include "alexa.h"

static bool     apMode      = false;
static bool     onlineReady = false;   // ja rodou NTP + mDNS depois de conectar
static uint32_t lostSince   = 0;
static time_t   slotTried[MAX_SLOTS] = {0};

// Um evento de alimentacao vai para todos os canais de aviso
static void avisaTodos(uint8_t portions, const char *src, bool ok, uint16_t pulses) {
    tgNotifyFeed(portions, src, ok, pulses);
    mqttNotifyFeed(portions, src, ok, pulses);
}

// --------------------------------------------------------------------- hora
static bool timeOk() {
    time_t now = time(nullptr);
    struct tm tmv;
    localtime_r(&now, &tmv);
    return (tmv.tm_year + 1900) >= 2024;
}

// --------------------------------------------------------------------- rede
// Chamado sempre que o Wi-Fi conecta (no boot ou numa reconexao): liga NTP e mDNS.
static void wifiOnConnected() {
    Serial.printf("[wifi] ok  ip=%s  rssi=%d dBm\n",
                  WiFi.localIP().toString().c_str(), WiFi.RSSI());
    configTzTime(cfg.tz, DEF_NTP1, DEF_NTP2);
    MDNS.end();
    if (MDNS.begin(cfg.host)) {
        MDNS.addService("http", "tcp", HTTP_PORT);
        Serial.printf("[wifi] http://%s.local\n", cfg.host);
    }
    onlineReady = true;
}

static void startWifi() {
    WiFi.persistent(false);
    WiFi.mode(WIFI_STA);
    WiFi.setHostname(cfg.host);
    WiFi.setSleep(false);          // sem isso o stream MJPEG engasga

    // Nunca configurado -> sobe o modo AP so para a configuracao inicial.
    if (!strlen(cfg.ssid)) {
        WiFi.mode(WIFI_AP);
        char ssid[32];
        snprintf(ssid, sizeof(ssid), AP_SSID_PREFIX "%04X",
                 (uint16_t)(ESP.getEfuseMac() & 0xFFFF));
        WiFi.softAP(ssid, AP_PASSWORD);
        apMode = true;
        Serial.printf("[wifi] MODO CONFIGURACAO -> rede \"%s\" senha \"%s\"\n", ssid, AP_PASSWORD);
        Serial.println(F("[wifi] abra http://192.168.4.1 e configure em Ajustes > Rede"));
        return;
    }

    // Ja configurado -> tenta a rede salva. Se falhar (ex.: roteador ainda subindo
    // apos queda de luz), NAO vira roteador: segue tentando em segundo plano (loop)
    // e a agenda continua rodando offline com o relogio interno.
    Serial.printf("[wifi] conectando em \"%s\"", cfg.ssid);
    WiFi.begin(cfg.ssid, cfg.pass);
    uint32_t t0 = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - t0 < WIFI_TIMEOUT_MS) {
        delay(250);
        Serial.print('.');
    }
    Serial.println();

    if (WiFi.status() == WL_CONNECTED)
        wifiOnConnected();
    else
        Serial.println(F("[wifi] sem rede no boot; sigo tentando e a agenda roda offline"));
}

// ---------------------------------------------------------------- agenda
static void schedTask(void *) {
    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(15000));
        if (!timeOk() || feederBusy()) continue;

        time_t now = time(nullptr);
        struct tm tmv;
        localtime_r(&now, &tmv);

        for (int i = 0; i < MAX_SLOTS; i++) {
            Slot &s = cfg.slots[i];
            if (!s.en) continue;
            if (!(s.dow & (1 << tmv.tm_wday))) continue;

            struct tm st = tmv;
            st.tm_hour = s.h;
            st.tm_min  = s.m;
            st.tm_sec  = 0;
            time_t slotT = mktime(&st);

            if (slotT > now) continue;                              // ainda nao deu a hora
            if (now - slotT > (time_t)cfg.catchUpMin * 60) continue; // fora da janela
            if (feederLastFeed() >= slotT) continue;                 // ja comeu depois disso
            if (slotTried[i] == slotT) continue;                     // ja tentou este horario

            slotTried[i] = slotT;
            String err;
            if (feederRequest(s.portions, SRC_SCHED, err))
                Serial.printf("[agenda] slot %d disparado (%02u:%02u)\n", i, s.h, s.m);
            else
                Serial.printf("[agenda] slot %d barrado: %s\n", i, err.c_str());
            break;                                                  // um por ciclo
        }
    }
}

// ----------------------------------------------------------------- botao
#if PIN_BUTTON >= 0
static void buttonTask(void *) {
    pinMode(PIN_BUTTON, INPUT_PULLUP);
    for (;;) {
        if (digitalRead(PIN_BUTTON) == LOW) {
            uint32_t t0 = millis();
            while (digitalRead(PIN_BUTTON) == LOW && millis() - t0 < 4000)
                vTaskDelay(pdMS_TO_TICKS(20));
            uint32_t held = millis() - t0;
            if (held > 700 && held < 4000) {
                String err;
                if (feederRequest(1, SRC_BUTTON, err))
                    Serial.println(F("[botao] 1 porcao"));
                else
                    Serial.printf("[botao] barrado: %s\n", err.c_str());
            }
            vTaskDelay(pdMS_TO_TICKS(500));
        }
        vTaskDelay(pdMS_TO_TICKS(60));
    }
}
#endif

// ----------------------------------------------------------------- setup
void setup() {
    Serial.begin(115200);
    delay(400);
    Serial.printf("\n\n=== AquaFeeder %s ===\n", FW_VERSION);
    Serial.printf("[sys] chip=%s psram=%u KB flash=%u MB\n",
                  ESP.getChipModel(), ESP.getPsramSize() / 1024,
                  ESP.getFlashChipSize() / (1024 * 1024));

    settingsLoad();
    feederInit();
    feederOnDone(avisaTodos);

#if HAS_CAMERA
    if (!cameraInit())
        Serial.println(F("[cam] seguindo sem camera (confira o cabo flat e a placa)"));
#endif

    startWifi();
    webStart();
    tgBegin();
    mqttBegin();
    alexaBegin();

    ArduinoOTA.setHostname(cfg.host);
    if (strlen(cfg.uiPass)) ArduinoOTA.setPassword(cfg.uiPass);
    ArduinoOTA.onStart([]() { Serial.println(F("[ota] atualizando...")); });
    ArduinoOTA.begin();

    xTaskCreatePinnedToCore(schedTask, "agenda", 4096, nullptr, 2, nullptr, 1);
#if PIN_BUTTON >= 0
    xTaskCreatePinnedToCore(buttonTask, "botao", 2560, nullptr, 1, nullptr, 1);
#endif

    if (!strlen(cfg.uiPass))
        Serial.println(F("[aviso] interface SEM SENHA. Defina uma em Ajustes > Rede."));
    Serial.println(F("[sys] pronto."));
}

// ------------------------------------------------------------------ loop
void loop() {
    ArduinoOTA.handle();

    if (webRebootRequested()) {
        Serial.println(F("[sys] reiniciando por pedido da interface"));
        delay(700);
        ESP.restart();
    }

    // vigia o Wi-Fi: reconecta em segundo plano e, ao voltar, religa NTP/mDNS.
    // Um aparelho ja configurado nunca vira roteador -- so segue tentando a rede.
    static uint32_t lastCheck = 0;
    if (!apMode && millis() - lastCheck > 15000) {
        lastCheck = millis();
        if (WiFi.status() == WL_CONNECTED) {
            lostSince = 0;
            if (!onlineReady) wifiOnConnected();     // conectou agora (boot offline ou reconexao)
        } else {
            if (!lostSince) lostSince = millis();
            onlineReady = false;                     // refaz NTP/mDNS quando a rede voltar
            Serial.println(F("[wifi] sem rede, tentando reconectar"));
            WiFi.disconnect();
            WiFi.begin(cfg.ssid, cfg.pass);
            if (millis() - lostSince > 600000UL) {   // 10 min sem rede -> reinicia
                Serial.println(F("[wifi] sem rede ha 10 min, reiniciando"));
                delay(200);
                ESP.restart();
            }
        }
    }
    delay(50);
}
