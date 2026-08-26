#include "settings.h"
#include <Preferences.h>

Settings cfg;
static Preferences prefs;

static void copyIf(JsonObjectConst o, const char *key, char *dst, size_t n) {
    if (o[key].is<const char *>()) strlcpy(dst, o[key].as<const char *>(), n);
}

void settingsDefaults() {
    memset(&cfg, 0, sizeof(cfg));
    strlcpy(cfg.host, DEF_HOSTNAME, sizeof(cfg.host));
    strlcpy(cfg.tz,   DEF_TZ,       sizeof(cfg.tz));
    cfg.stepsPerPortion = DEF_STEPS_PER_PORTION;
    cfg.stepUs          = DEF_STEP_US;
    cfg.reverse         = false;
    cfg.maxPerDay       = DEF_MAX_PER_DAY;
    cfg.maxPerRequest   = DEF_MAX_PER_REQUEST;
    cfg.minIntervalS    = DEF_MIN_INTERVAL_S;
    cfg.catchUpMin      = DEF_CATCHUP_MIN;
    cfg.sensorEnabled   = false;
    cfg.tgNotify        = true;
    cfg.alexaPortions   = 1;
    cfg.camVflip        = false;
    cfg.camHmirror      = false;
    cfg.camSize         = 2;      // SVGA 800x600
    cfg.camQuality      = 12;
    cfg.mqttPort        = 8883;
    cfg.mqttTls         = true;
    strlcpy(cfg.mqttPrefix, "aquafeeder", sizeof(cfg.mqttPrefix));
    // agenda inicial: 08:00 e 18:00, 2 porcoes, todos os dias
    cfg.slots[0] = {true, 8,  0, 0x7F, 2};
    cfg.slots[1] = {true, 18, 0, 0x7F, 2};
}

void settingsLoad() {
    settingsDefaults();
    prefs.begin("aquafeeder", true);
    String json = prefs.getString("cfg", "");
    prefs.end();
    if (!json.length()) {
        Serial.println(F("[cfg] sem configuracao salva, usando padroes"));
        return;
    }
    JsonDocument doc;
    if (deserializeJson(doc, json)) {
        Serial.println(F("[cfg] JSON invalido na NVS, usando padroes"));
        return;
    }
    settingsFromJson(doc.as<JsonObjectConst>());
    Serial.printf("[cfg] carregado (%u bytes)\n", json.length());
}

void settingsSave() {
    JsonDocument doc;
    JsonObject o = doc.to<JsonObject>();
    settingsToJson(o, true);
    String json;
    serializeJson(doc, json);
    prefs.begin("aquafeeder", false);
    prefs.putString("cfg", json);
    prefs.end();
    Serial.printf("[cfg] salvo (%u bytes)\n", json.length());
}

void settingsToJson(JsonObject o, bool secrets) {
    o["ssid"] = cfg.ssid;
    o["host"] = cfg.host;
    o["tz"]   = cfg.tz;
    o["hasUiPass"]   = strlen(cfg.uiPass) > 0;
    o["hasWifiPass"] = strlen(cfg.pass) > 0;
    o["tgEnabled"]  = strlen(cfg.tgToken) > 0;
    o["tgChat"]     = cfg.tgChat;
    o["tgNotify"]   = cfg.tgNotify;
    o["alexaEnabled"]  = strlen(cfg.alexaDevId) > 10;
    o["alexaPortions"] = cfg.alexaPortions;
    o["stepsPerPortion"] = cfg.stepsPerPortion;
    o["stepUs"]          = cfg.stepUs;
    o["reverse"]         = cfg.reverse;
    o["maxPerDay"]       = cfg.maxPerDay;
    o["maxPerRequest"]   = cfg.maxPerRequest;
    o["minIntervalS"]    = cfg.minIntervalS;
    o["catchUpMin"]      = cfg.catchUpMin;
    o["sensorEnabled"]   = cfg.sensorEnabled;
    o["camVflip"]        = cfg.camVflip;
    o["camHmirror"]      = cfg.camHmirror;
    o["camSize"]         = cfg.camSize;
    o["camQuality"]      = cfg.camQuality;
    o["camExtUrl"]       = cfg.camExtUrl;
    o["mqttHost"]        = cfg.mqttHost;
    o["mqttPort"]        = cfg.mqttPort;
    o["mqttUser"]        = cfg.mqttUser;
    o["mqttPrefix"]      = cfg.mqttPrefix;
    o["mqttTls"]         = cfg.mqttTls;
    o["mqttEnabled"]     = strlen(cfg.mqttHost) > 3;
    o["mqttHasPass"]     = strlen(cfg.mqttPass) > 0;
    if (secrets) {
        o["pass"]    = cfg.pass;
        o["uiPass"]  = cfg.uiPass;
        o["tgToken"]   = cfg.tgToken;
        o["mqttPass"] = cfg.mqttPass;
        o["alexaAppKey"] = cfg.alexaAppKey;
        o["alexaSecret"] = cfg.alexaSecret;
        o["alexaDevId"]  = cfg.alexaDevId;
    }
    JsonArray arr = o["slots"].to<JsonArray>();
    for (int i = 0; i < MAX_SLOTS; i++) {
        JsonObject s = arr.add<JsonObject>();
        s["en"]  = cfg.slots[i].en;
        s["h"]   = cfg.slots[i].h;
        s["m"]   = cfg.slots[i].m;
        s["dow"] = cfg.slots[i].dow;
        s["p"]   = cfg.slots[i].portions;
    }
}

bool settingsFromJson(JsonObjectConst o) {
    copyIf(o, "ssid",    cfg.ssid,    sizeof(cfg.ssid));
    copyIf(o, "pass",    cfg.pass,    sizeof(cfg.pass));
    copyIf(o, "host",    cfg.host,    sizeof(cfg.host));
    copyIf(o, "uiPass",  cfg.uiPass,  sizeof(cfg.uiPass));
    copyIf(o, "tz",      cfg.tz,      sizeof(cfg.tz));
    copyIf(o, "camExtUrl",  cfg.camExtUrl,  sizeof(cfg.camExtUrl));
    copyIf(o, "mqttHost",   cfg.mqttHost,   sizeof(cfg.mqttHost));
    copyIf(o, "mqttUser",   cfg.mqttUser,   sizeof(cfg.mqttUser));
    copyIf(o, "mqttPass",   cfg.mqttPass,   sizeof(cfg.mqttPass));
    copyIf(o, "mqttPrefix", cfg.mqttPrefix, sizeof(cfg.mqttPrefix));
    copyIf(o, "tgToken", cfg.tgToken, sizeof(cfg.tgToken));
    copyIf(o, "tgChat",  cfg.tgChat,  sizeof(cfg.tgChat));
    copyIf(o, "alexaAppKey", cfg.alexaAppKey, sizeof(cfg.alexaAppKey));
    copyIf(o, "alexaSecret", cfg.alexaSecret, sizeof(cfg.alexaSecret));
    copyIf(o, "alexaDevId",  cfg.alexaDevId,  sizeof(cfg.alexaDevId));

    if (o["tgNotify"].is<bool>())      cfg.tgNotify      = o["tgNotify"];
    if (o["reverse"].is<bool>())       cfg.reverse       = o["reverse"];
    if (o["sensorEnabled"].is<bool>()) cfg.sensorEnabled = o["sensorEnabled"];
    if (o["camVflip"].is<bool>())      cfg.camVflip      = o["camVflip"];
    if (o["camHmirror"].is<bool>())    cfg.camHmirror    = o["camHmirror"];
    if (o["mqttTls"].is<bool>())       cfg.mqttTls       = o["mqttTls"];
    if (o["mqttPort"].is<uint16_t>())  cfg.mqttPort      = o["mqttPort"];
    if (o["camSize"].is<uint8_t>())    cfg.camSize       = constrain((int)o["camSize"], 0, 3);
    if (o["camQuality"].is<uint8_t>()) cfg.camQuality    = constrain((int)o["camQuality"], 10, 30);
    if (o["alexaPortions"].is<uint8_t>()) cfg.alexaPortions = constrain((int)o["alexaPortions"], 1, 20);

    if (o["stepsPerPortion"].is<uint16_t>())
        cfg.stepsPerPortion = constrain((int)o["stepsPerPortion"], 40, 20000);
    if (o["stepUs"].is<uint16_t>())
        cfg.stepUs = constrain((int)o["stepUs"], 1000, 8000);
    if (o["maxPerDay"].is<uint8_t>())
        cfg.maxPerDay = constrain((int)o["maxPerDay"], 1, 60);
    if (o["maxPerRequest"].is<uint8_t>())
        cfg.maxPerRequest = constrain((int)o["maxPerRequest"], 1, 20);
    if (o["minIntervalS"].is<uint16_t>())
        cfg.minIntervalS = constrain((int)o["minIntervalS"], 0, 3600);
    if (o["catchUpMin"].is<uint16_t>())
        cfg.catchUpMin = constrain((int)o["catchUpMin"], 0, 720);

    if (o["slots"].is<JsonArrayConst>()) {
        JsonArrayConst arr = o["slots"];
        int i = 0;
        for (JsonObjectConst s : arr) {
            if (i >= MAX_SLOTS) break;
            cfg.slots[i].en       = s["en"] | false;
            cfg.slots[i].h        = constrain((int)(s["h"] | 0), 0, 23);
            cfg.slots[i].m        = constrain((int)(s["m"] | 0), 0, 59);
            cfg.slots[i].dow      = (s["dow"] | 0x7F) & 0x7F;
            cfg.slots[i].portions = constrain((int)(s["p"] | 1), 1, 20);
            i++;
        }
        for (; i < MAX_SLOTS; i++) cfg.slots[i].en = false;
    }
    return true;
}
