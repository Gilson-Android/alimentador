#include "webserver.h"
#include "webui.h"
#include "settings.h"
#include "feeder.h"
#include "camera.h"
#include <WiFi.h>
#include <esp_http_server.h>
#include <esp_random.h>
#if HAS_CAMERA
#include "esp_camera.h"
#endif

static httpd_handle_t srv = nullptr;
static httpd_handle_t srvStream = nullptr;
static bool  rebootFlag = false;
static char  sessions[4][33] = {{0}, {0}, {0}, {0}};
static uint8_t sessNext = 0;
static char  cookieHdr[80];

// ------------------------------------------------------------- utilidades
void fmtWhen(time_t t, char *out, size_t n) {
    if (t < 1700000000) { strlcpy(out, "-", n); return; }
    struct tm tmv;
    localtime_r(&t, &tmv);
    strftime(out, n, "%d/%m %H:%M", &tmv);
}

static bool timeSynced() {
    time_t now = time(nullptr);
    struct tm tmv;
    localtime_r(&now, &tmv);
    return (tmv.tm_year + 1900) >= 2024;
}

static esp_err_t sendJson(httpd_req_t *r, JsonDocument &doc, int code = 200) {
    String out;
    serializeJson(doc, out);
    // atencao: a string de status precisa continuar valida ate o envio
    static char st[32];
    if (code != 200) {
        const char *reason = code == 401 ? "Unauthorized"
                           : code == 429 ? "Too Many Requests"
                           : code == 503 ? "Service Unavailable"
                                         : "Bad Request";
        snprintf(st, sizeof(st), "%d %s", code, reason);
        httpd_resp_set_status(r, st);
    }
    httpd_resp_set_type(r, "application/json; charset=utf-8");
    httpd_resp_set_hdr(r, "Cache-Control", "no-store");
    return httpd_resp_sendstr(r, out.c_str());
}

static esp_err_t sendErr(httpd_req_t *r, int code, const String &msg) {
    JsonDocument d;
    d["err"] = msg;
    return sendJson(r, d, code);
}

static bool readBody(httpd_req_t *r, JsonDocument &doc) {
    if (r->content_len == 0 || r->content_len > 4096) return false;
    String body;
    body.reserve(r->content_len + 1);
    char buf[513];
    size_t left = r->content_len;
    while (left) {
        int n = httpd_req_recv(r, buf, left > 512 ? 512 : left);
        if (n <= 0) return false;
        buf[n] = 0;
        body += buf;
        left -= n;
    }
    return deserializeJson(doc, body) == DeserializationError::Ok;
}

static int qInt(httpd_req_t *r, const char *key, int def) {
    char q[160], v[24];
    if (httpd_req_get_url_query_str(r, q, sizeof(q)) != ESP_OK) return def;
    if (httpd_query_key_value(q, key, v, sizeof(v)) != ESP_OK) return def;
    return atoi(v);
}

// ------------------------------------------------------------------ sessao
static bool authed(httpd_req_t *r) {
    if (!strlen(cfg.uiPass)) return true;            // sem senha configurada
    char tok[40];
    size_t len = sizeof(tok);
    if (httpd_req_get_cookie_val(r, "af", tok, &len) != ESP_OK) return false;
    for (auto &s : sessions)
        if (s[0] && strcmp(s, tok) == 0) return true;
    return false;
}

static void newSession(char *out, size_t n) {
    uint32_t a = esp_random(), b = esp_random(), c = esp_random(), d = esp_random();
    snprintf(out, n, "%08x%08x%08x%08x", a, b, c, d);
}

#define REQUIRE_AUTH(r) if (!authed(r)) return sendErr(r, 401, F("Faça login."))

// ================================================================ handlers
static esp_err_t hIndex(httpd_req_t *r) {
    httpd_resp_set_type(r, "text/html; charset=utf-8");
    httpd_resp_set_hdr(r, "Cache-Control", "no-store");
    return httpd_resp_send(r, INDEX_HTML, strlen(INDEX_HTML));
}

static esp_err_t hFavicon(httpd_req_t *r) {
    httpd_resp_set_status(r, "204 No Content");
    return httpd_resp_send(r, nullptr, 0);
}

static esp_err_t hLogin(httpd_req_t *r) {
    JsonDocument d;
    if (!readBody(r, d)) return sendErr(r, 400, F("corpo inválido"));
    const char *p = d["pass"] | "";
    if (strlen(cfg.uiPass) && strcmp(p, cfg.uiPass) != 0)
        return sendErr(r, 401, F("senha incorreta"));
    char tok[33];
    newSession(tok, sizeof(tok));
    strlcpy(sessions[sessNext], tok, 33);
    sessNext = (sessNext + 1) % 4;
    snprintf(cookieHdr, sizeof(cookieHdr),
             "af=%s; Path=/; Max-Age=2592000; SameSite=Lax", tok);
    httpd_resp_set_hdr(r, "Set-Cookie", cookieHdr);
    JsonDocument ok;
    ok["ok"] = true;
    return sendJson(r, ok);
}

static esp_err_t hStatus(httpd_req_t *r) {
    REQUIRE_AUTH(r);
    JsonDocument d;
    bool sta = WiFi.status() == WL_CONNECTED;
    d["wifi"]  = sta;
    d["ip"]    = sta ? WiFi.localIP().toString() : WiFi.softAPIP().toString();
    d["rssi"]  = sta ? WiFi.RSSI() : 0;
    d["ap"]    = !sta;
    char w[24];
    if (timeSynced()) {
        time_t now = time(nullptr);
        struct tm tmv;
        localtime_r(&now, &tmv);
        strftime(w, sizeof(w), "%H:%M", &tmv);
        d["time"] = w;
    } else {
        d["time"] = "sem hora";
    }
    fmtWhen(feederLastFeed(), w, sizeof(w));
    d["lastFeed"]  = w;
    d["fedToday"]  = feederFedToday();
    d["maxPerDay"] = cfg.maxPerDay;
    d["busy"]      = feederBusy();
    d["total"]     = feederTotalPortions();
    d["cam"]       = cameraReady();
    d["camErr"]    = cameraLastError();
    d["heap"]      = ESP.getFreeHeap();
    d["up"]        = (uint32_t)(millis() / 1000);
    d["fw"]        = FW_VERSION;
    return sendJson(r, d);
}

static esp_err_t hGetConfig(httpd_req_t *r) {
    REQUIRE_AUTH(r);
    // ?secrets=1 devolve token/senhas (so pra quem ja esta autenticado, pro
    // botao "Copiar" da interface). Sem isso, os segredos nunca saem do aparelho.
    JsonDocument d;
    JsonObject o = d.to<JsonObject>();
    settingsToJson(o, qInt(r, "secrets", 0) == 1);
    return sendJson(r, d);
}

static esp_err_t hPostConfig(httpd_req_t *r) {
    REQUIRE_AUTH(r);
    JsonDocument d;
    if (!readBody(r, d)) return sendErr(r, 400, F("corpo inválido"));

    char oldSsid[33], oldHost[24];
    strlcpy(oldSsid, cfg.ssid, sizeof(oldSsid));
    strlcpy(oldHost, cfg.host, sizeof(oldHost));
    bool wifiPass = d["pass"].is<const char *>();
    // As tasks de Telegram/MQTT/Alexa so leem suas chaves no boot. Se o usuario
    // acabou de colar uma chave da Alexa, reinicia para a conexao subir na hora.
    bool alexaKeys = d["alexaAppKey"].is<const char *>() ||
                     d["alexaSecret"].is<const char *>() ||
                     d["alexaDevId"].is<const char *>();

    settingsFromJson(d.as<JsonObjectConst>());
    settingsSave();
    cameraApplySettings();
    setenv("TZ", cfg.tz, 1);
    tzset();

    bool needReboot = wifiPass || alexaKeys ||
                      strcmp(oldSsid, cfg.ssid) != 0 || strcmp(oldHost, cfg.host) != 0;
    JsonDocument ok;
    ok["ok"] = true;
    ok["reboot"] = needReboot;
    esp_err_t e = sendJson(r, ok);
    if (needReboot) rebootFlag = true;
    return e;
}

static esp_err_t hFeed(httpd_req_t *r) {
    REQUIRE_AUTH(r);
    int p = qInt(r, "p", 1);
    if (p < 1) p = 1;
    String err;
    if (!feederRequest((uint8_t)p, SRC_MANUAL, err)) return sendErr(r, 429, err);
    JsonDocument ok;
    ok["ok"] = true;
    return sendJson(r, ok);
}

static esp_err_t hJog(httpd_req_t *r) {
    REQUIRE_AUTH(r);
    int s = qInt(r, "s", 0);
    if (s == 0) return sendErr(r, 400, F("s ausente"));
    if (feederBusy()) return sendErr(r, 429, F("motor ocupado"));
    feederJog(s);
    JsonDocument ok;
    ok["ok"] = true;
    return sendJson(r, ok);
}

static esp_err_t hLog(httpd_req_t *r) {
    REQUIRE_AUTH(r);
    FeedEntry buf[LOG_SIZE];
    size_t n = feederLogCopy(buf, LOG_SIZE);
    JsonDocument d;
    JsonArray a = d["items"].to<JsonArray>();
    char w[24];
    for (size_t i = 0; i < n; i++) {
        JsonObject o = a.add<JsonObject>();
        fmtWhen(buf[i].ts, w, sizeof(w));
        o["when"]   = w;
        o["p"]      = buf[i].portions;
        o["src"]    = feedSrcName(buf[i].src);
        o["pulses"] = buf[i].pulses;
        o["ok"]     = buf[i].ok;
    }
    return sendJson(r, d);
}

static esp_err_t hReboot(httpd_req_t *r) {
    REQUIRE_AUTH(r);
    JsonDocument ok;
    ok["ok"] = true;
    esp_err_t e = sendJson(r, ok);
    rebootFlag = true;
    return e;
}

// varre as redes proximas (so 2,4 GHz -- e o unico radio do ESP32) pra
// preencher o SSID na tela de configuracao sem digitar
static esp_err_t hScan(httpd_req_t *r) {
    REQUIRE_AUTH(r);
    // habilita a interface STA sem derrubar o AP, so pra varrer
    if (WiFi.getMode() == WIFI_MODE_AP) WiFi.mode(WIFI_MODE_APSTA);
    int n = WiFi.scanNetworks();
    JsonDocument d;
    JsonArray a = d["nets"].to<JsonArray>();
    for (int i = 0; i < n; i++) {
        String s = WiFi.SSID(i);
        if (s.isEmpty()) continue;                        // rede oculta
        bool dup = false;                                 // mesma rede em 2 canais
        for (JsonObject o : a)
            if (s == o["ssid"].as<const char *>()) { dup = true; break; }
        if (dup) continue;
        JsonObject o = a.add<JsonObject>();
        o["ssid"] = s;
        o["rssi"] = WiFi.RSSI(i);
        o["lock"] = WiFi.encryptionType(i) != WIFI_AUTH_OPEN;
    }
    WiFi.scanDelete();
    return sendJson(r, d);
}

// ------------------------------------------------------------------ camera
#if HAS_CAMERA
static esp_err_t hSnapshot(httpd_req_t *r) {
    REQUIRE_AUTH(r);
    if (!cameraReady()) return sendErr(r, 503, F("câmera indisponível"));
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) return sendErr(r, 503, F("falha ao capturar"));
    httpd_resp_set_type(r, "image/jpeg");
    httpd_resp_set_hdr(r, "Cache-Control", "no-store");
    httpd_resp_set_hdr(r, "Content-Disposition", "inline; filename=aquario.jpg");
    esp_err_t e = httpd_resp_send(r, (const char *)fb->buf, fb->len);
    esp_camera_fb_return(fb);
    return e;
}

#define BOUND "afframe"
static const char *ST_TYPE  = "multipart/x-mixed-replace;boundary=" BOUND;
static const char *ST_BOUND = "\r\n--" BOUND "\r\n";
static const char *ST_PART  = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

static esp_err_t hStream(httpd_req_t *r) {
    REQUIRE_AUTH(r);
    if (!cameraReady()) return sendErr(r, 503, F("câmera indisponível"));
    esp_err_t res = httpd_resp_set_type(r, ST_TYPE);
    if (res != ESP_OK) return res;
    httpd_resp_set_hdr(r, "Cache-Control", "no-store");
    httpd_resp_set_hdr(r, "X-Framerate", "10");

    char part[80];
    while (true) {
        camera_fb_t *fb = esp_camera_fb_get();
        if (!fb) { res = ESP_FAIL; break; }
        size_t hlen = snprintf(part, sizeof(part), ST_PART, fb->len);
        res = httpd_resp_send_chunk(r, ST_BOUND, strlen(ST_BOUND));
        if (res == ESP_OK) res = httpd_resp_send_chunk(r, part, hlen);
        if (res == ESP_OK) res = httpd_resp_send_chunk(r, (const char *)fb->buf, fb->len);
        esp_camera_fb_return(fb);
        if (res != ESP_OK) break;
        vTaskDelay(pdMS_TO_TICKS(100));      // ~10 fps, poupa Wi-Fi e calor
    }
    return res;
}
#endif

// =================================================================== setup
static void reg(httpd_handle_t h, const char *uri, httpd_method_t m, esp_err_t (*fn)(httpd_req_t *)) {
    httpd_uri_t u = {};
    u.uri = uri;
    u.method = m;
    u.handler = fn;
    httpd_register_uri_handler(h, &u);
}

void webStart() {
    httpd_config_t c = HTTPD_DEFAULT_CONFIG();
    c.server_port      = HTTP_PORT;
    c.ctrl_port        = 32768;
    c.max_uri_handlers = 16;
    c.max_open_sockets = 4;      // lwIP do Arduino tem so 10 sockets no total
    c.stack_size       = 8192;
    c.lru_purge_enable = true;
    if (httpd_start(&srv, &c) == ESP_OK) {
        reg(srv, "/",              HTTP_GET,  hIndex);
        reg(srv, "/favicon.ico",   HTTP_GET,  hFavicon);
        reg(srv, "/api/status",    HTTP_GET,  hStatus);
        reg(srv, "/api/config",    HTTP_GET,  hGetConfig);
        reg(srv, "/api/config",    HTTP_POST, hPostConfig);
        reg(srv, "/api/login",     HTTP_POST, hLogin);
        reg(srv, "/api/feed",      HTTP_POST, hFeed);
        reg(srv, "/api/jog",       HTTP_POST, hJog);
        reg(srv, "/api/log",       HTTP_GET,  hLog);
        reg(srv, "/api/reboot",    HTTP_POST, hReboot);
        reg(srv, "/api/scan",      HTTP_GET,  hScan);
#if HAS_CAMERA
        reg(srv, "/snapshot.jpg",  HTTP_GET,  hSnapshot);
#endif
        Serial.printf("[web] http://%s.local (porta %d)\n", cfg.host, HTTP_PORT);
    } else {
        Serial.println(F("[web] falha ao iniciar httpd"));
    }

#if HAS_CAMERA
    // stream em porta separada: um cliente de video nao trava a API
    httpd_config_t s = HTTPD_DEFAULT_CONFIG();
    s.server_port      = STREAM_PORT;
    s.ctrl_port        = 32769;
    s.max_uri_handlers = 2;
    s.max_open_sockets = 2;
    s.stack_size       = 8192;
    s.lru_purge_enable = true;
    if (httpd_start(&srvStream, &s) == ESP_OK) {
        reg(srvStream, "/stream", HTTP_GET, hStream);
        reg(srvStream, "/",       HTTP_GET, hStream);
    }
#endif
}

void webStop() {
    if (srv)       { httpd_stop(srv);       srv = nullptr; }
    if (srvStream) { httpd_stop(srvStream); srvStream = nullptr; }
}

bool webRebootRequested() { return rebootFlag; }
