// =====================================================================
//  AquaFeeder -- configuracao persistida em NVS (JSON)
// =====================================================================
#pragma once
#include <ArduinoJson.h>
#include "config.h"

// dow: bit0 = domingo ... bit6 = sabado.  0x7F = todos os dias
struct Slot {
    bool    en;
    uint8_t h;
    uint8_t m;
    uint8_t dow;
    uint8_t portions;
};

struct Settings {
    char     ssid[33];
    char     pass[65];
    char     host[24];
    char     uiPass[33];        // vazio = interface sem senha (so na LAN!)
    char     tz[40];
    // Camera externa (celular velho com IP Webcam, camera Wi-Fi, DVR...).
    // Se preenchido, a interface mostra ESSA imagem em vez da camera do ESP32.
    // O navegador busca a URL direto -- o ESP32 nao faz proxy, nao gasta RAM.
    char     camExtUrl[96];
    // MQTT (opcional): comando pela nuvem sem abrir porta no roteador.
    // Quem conecta e o ESP32, para fora. Broker vazio = MQTT desligado.
    char     mqttHost[64];
    uint16_t mqttPort;
    char     mqttUser[40];
    char     mqttPass[56];
    char     mqttPrefix[24];    // padrao "aquafeeder"
    bool     mqttTls;

    char     tgToken[64];       // token do bot do Telegram (opcional)
    char     tgChat[24];        // chat id autorizado
    bool     tgNotify;          // avisar a cada alimentacao

    // Alexa via Sinric Pro (nuvem gratuita). O ESP32 conecta para fora, como
    // no Telegram. Vazio = Alexa desligada. Veja alexa.h para o passo a passo.
    char     alexaAppKey[40];   // Sinric Pro "App Key"    (UUID, 36 chars)
    char     alexaSecret[80];   // Sinric Pro "App Secret" (73 chars)
    char     alexaDevId[40];    // ID do dispositivo criado no painel do Sinric
    uint8_t  alexaPortions;     // porcoes por "alimentar agora" (padrao 1)
    uint16_t stepsPerPortion;
    uint16_t stepUs;
    bool     reverse;           // inverte o sentido de giro
    uint8_t  maxPerDay;
    uint8_t  maxPerRequest;
    uint16_t minIntervalS;
    uint16_t catchUpMin;
    bool     sensorEnabled;
    bool     camVflip;
    bool     camHmirror;
    uint8_t  camSize;           // 0=QVGA 1=VGA 2=SVGA 3=HD
    uint8_t  camQuality;        // 10 (melhor) .. 30 (pior)
    Slot     slots[MAX_SLOTS];
};

extern Settings cfg;

void settingsDefaults();
void settingsLoad();
void settingsSave();
void settingsToJson(JsonObject o, bool includeSecrets);
bool settingsFromJson(JsonObjectConst o);
