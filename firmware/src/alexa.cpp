#include "alexa.h"
#include "settings.h"
#include "feeder.h"
#include <WiFi.h>
#include <SinricPro.h>
#include <SinricProSwitch.h>

static SinricProSwitch *sw = nullptr;
static volatile uint32_t offReportAt = 0;   // quando reportar "desligado" de volta

bool alexaEnabled() {
    return strlen(cfg.alexaAppKey) > 10 &&
           strlen(cfg.alexaSecret) > 10 &&
           strlen(cfg.alexaDevId)  > 10;
}

// A Alexa mandou ligar/desligar. So "ligar" alimenta; "desligar" e ignorado.
static bool onPowerState(const String &deviceId, bool &state) {
    if (!state) return true;                        // "desligar" nao faz nada
    String  err;
    uint8_t p = cfg.alexaPortions ? cfg.alexaPortions : 1;
    if (feederRequest(p, SRC_ALEXA, err))
        Serial.printf("[alexa] alimentando %u porcao(oes)\n", p);
    else
        Serial.printf("[alexa] barrado: %s\n", err.c_str());
    // Botao momentaneo: volta a aparecer "desligado" logo depois, para que o
    // proximo "ligar" (da rotina "alimentar agora") sempre dispare de novo.
    offReportAt = millis() + 1500;
    return true;                                    // confirma o comando a Alexa
}

static void tarefa(void *) {
    // WebSocket seguro so depois que a rede subir
    while (WiFi.status() != WL_CONNECTED) vTaskDelay(pdMS_TO_TICKS(500));
    vTaskDelay(pdMS_TO_TICKS(1500));

    SinricProSwitch &mySwitch = SinricPro[cfg.alexaDevId];
    sw = &mySwitch;
    mySwitch.onPowerState(onPowerState);

    SinricPro.onConnected([]()    { Serial.println(F("[alexa] conectado ao Sinric Pro")); });
    SinricPro.onDisconnected([]() { Serial.println(F("[alexa] desconectado do Sinric Pro")); });
    Serial.printf("[alexa] conectando (device %s)\n", cfg.alexaDevId);
    SinricPro.begin(cfg.alexaAppKey, cfg.alexaSecret);

    for (;;) {
        SinricPro.handle();
        if (offReportAt && millis() >= offReportAt) {
            offReportAt = 0;
            if (sw) sw->sendPowerStateEvent(false);
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void alexaBegin() {
    if (!alexaEnabled()) {
        Serial.println(F("[alexa] desligado (sem chaves do Sinric Pro)"));
        return;
    }
    xTaskCreatePinnedToCore(tarefa, "alexa", 12288, nullptr, 1, nullptr, 0);
    Serial.println(F("[alexa] task iniciada -> Sinric Pro"));
}
