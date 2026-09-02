#include "telegram.h"
#include "settings.h"
#include "feeder.h"
#include "camera.h"
#include "webserver.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include "mbedtls/base64.h"
#if HAS_CAMERA
#include "esp_camera.h"
#endif

static const char *TG_HOST = "api.telegram.org";
static const int   TG_PORT = 443;
static int64_t     tgOffset = 0;
static bool        drained  = false;

bool tgEnabled() { return strlen(cfg.tgToken) > 4; }

// ------------------------------------------------------------------ helpers
// Abre TLS. Usamos setInsecure(): nao validamos o certificado do Telegram.
// Para um aparelho domestico que so FALA com api.telegram.org isso e um risco
// aceitavel (e o que praticamente todas as bibliotecas de bot fazem). Se
// quiser validar, cole o certificado raiz em TG_ROOT_CA e use setCACert().
static bool tgConnect(WiFiClientSecure &cli, uint32_t timeoutMs = 12000) {
    cli.setInsecure();
    cli.setTimeout(timeoutMs / 1000);
    return cli.connect(TG_HOST, TG_PORT, timeoutMs);
}

static bool skipHeaders(WiFiClientSecure &cli, uint32_t timeoutMs) {
    uint32_t t0 = millis();
    while (cli.connected() && millis() - t0 < timeoutMs) {
        String line = cli.readStringUntil('\n');
        if (line == "\r" || line.length() == 0) return true;
    }
    return false;
}

static bool tgPostJson(const char *method, const String &body, JsonDocument *out = nullptr,
                       JsonDocument *filter = nullptr, uint32_t timeoutMs = 12000) {
    if (!tgEnabled() || WiFi.status() != WL_CONNECTED) return false;
    WiFiClientSecure cli;
    if (!tgConnect(cli, timeoutMs)) {
        Serial.println(F("[tg] falha de conexao TLS"));
        return false;
    }
    String req;
    req.reserve(200 + body.length());
    req  = "POST /bot"; req += cfg.tgToken; req += "/"; req += method; req += " HTTP/1.1\r\n";
    req += "Host: "; req += TG_HOST; req += "\r\n";
    req += "Content-Type: application/json\r\n";
    req += "Content-Length: "; req += body.length(); req += "\r\n";
    req += "Connection: close\r\n\r\n";
    req += body;
    cli.print(req);

    bool ok = skipHeaders(cli, timeoutMs);
    if (ok && out) {
        DeserializationError e = filter
            ? deserializeJson(*out, cli, DeserializationOption::Filter(*filter))
            : deserializeJson(*out, cli);
        ok = (e == DeserializationError::Ok);
        if (!ok) Serial.printf("[tg] json: %s\n", e.c_str());
    }
    cli.stop();
    return ok;
}

static bool tgSend(const String &text) {
    if (!strlen(cfg.tgChat)) return false;
    JsonDocument d;
    d["chat_id"] = cfg.tgChat;
    d["text"]    = text;
    String body;
    serializeJson(d, body);
    return tgPostJson("sendMessage", body);
}

// Retrato dos ajustes OPERACIONAIS (sem rede/segredos) para o painel do
// Telegram pre-preencher os campos. Vai embutido no link do botao "Abrir
// painel" -- e o unico jeito de o Mini App (hospedado fora) LER o estado atual
// sem servidor nenhum. Mantido enxuto para o link nao ficar gigante.
static String cfgSnapshotJson() {
    JsonDocument d;
    d["stepsPerPortion"] = cfg.stepsPerPortion;
    d["stepUs"]          = cfg.stepUs;
    d["reverse"]         = cfg.reverse;
    d["sensorEnabled"]   = cfg.sensorEnabled;
    d["antiJam"]         = cfg.antiJam;
    d["shakeCycles"]     = cfg.shakeCycles;
    d["maxPerDay"]       = cfg.maxPerDay;
    d["maxPerRequest"]   = cfg.maxPerRequest;
    d["minIntervalS"]    = cfg.minIntervalS;
    d["catchUpMin"]      = cfg.catchUpMin;
    d["camVflip"]        = cfg.camVflip;
    d["camHmirror"]      = cfg.camHmirror;
    d["camSize"]         = cfg.camSize;
    d["camQuality"]      = cfg.camQuality;
    d["tgNotify"]        = cfg.tgNotify;
    d["alexaPortions"]   = cfg.alexaPortions;
    d["tz"]              = cfg.tz;
    d["hasCam"]          = HAS_CAMERA ? 1 : 0;
    JsonArray a = d["slots"].to<JsonArray>();
    for (int i = 0; i < MAX_SLOTS; i++) {
        JsonObject s = a.add<JsonObject>();
        s["en"]  = cfg.slots[i].en;
        s["h"]   = cfg.slots[i].h;
        s["m"]   = cfg.slots[i].m;
        s["dow"] = cfg.slots[i].dow;
        s["p"]   = cfg.slots[i].portions;
    }
    String out;
    serializeJson(d, out);
    return out;
}

// base64url (sem padding) do retrato -- seguro para colocar numa query string
static String cfgSnapshotB64() {
    String json = cfgSnapshotJson();
    size_t need = 0;
    mbedtls_base64_encode(nullptr, 0, &need,
                          (const unsigned char *)json.c_str(), json.length());
    char *buf = (char *)malloc(need + 1);
    if (!buf) return String();
    size_t olen = 0;
    String out;
    if (mbedtls_base64_encode((unsigned char *)buf, need + 1, &olen,
                              (const unsigned char *)json.c_str(), json.length()) == 0) {
        out.reserve(olen);
        for (size_t i = 0; i < olen; i++) {
            char ch = buf[i];
            if (ch == '+') ch = '-';
            else if (ch == '/') ch = '_';
            else if (ch == '=') continue;   // sem padding
            out += ch;
        }
    }
    free(buf);
    return out;
}

// Envia uma mensagem JUNTO com o teclado de botoes. O Telegram fixa esse
// teclado na conversa (is_persistent) ate ser trocado -- entao basta mandar
// uma vez (no /start ou /menu) para os botoes ficarem sempre disponiveis.
// Tocar num botao envia o rotulo como texto normal, que handleCommand traduz.
static bool tgSendKb(const String &text) {
    if (!strlen(cfg.tgChat)) return false;
    JsonDocument d;
    d["chat_id"] = cfg.tgChat;
    d["text"]    = text;
    JsonObject rm = d["reply_markup"].to<JsonObject>();
    rm["resize_keyboard"] = true;
    rm["is_persistent"]   = true;
    JsonArray kb = rm["keyboard"].to<JsonArray>();
    // Botao que abre o painel (Mini App). So aparece se a URL foi definida em
    // config.h. Ao tocar num comando no painel, ele volta para o bot via
    // web_app_data (tratado em pollOnce) -- sem broker, tudo pelo Telegram.
    if (strlen(TG_WEBAPP_URL) > 0) {
        JsonArray r0 = kb.add<JsonArray>();
        JsonObject b = r0.add<JsonObject>();
        b["text"] = "🐟 Abrir painel";
        // Anexa o retrato da config na URL para o painel abrir ja preenchido.
        String url  = TG_WEBAPP_URL;
        String snap = cfgSnapshotB64();
        if (snap.length()) {
            url += (url.indexOf('?') >= 0) ? "&c=" : "?c=";
            url += snap;
        }
        b["web_app"]["url"] = url;
    }
    JsonArray r1 = kb.add<JsonArray>();
    r1.add("🐟 Alimentar"); r1.add("🐟 Alimentar 2x");
    JsonArray r2 = kb.add<JsonArray>();
    r2.add("📷 Foto"); r2.add("📊 Status");
    JsonArray r3 = kb.add<JsonArray>();
    r3.add("📅 Agenda");
    String body;
    serializeJson(d, body);

    // Le a resposta da API: se o Telegram recusar o teclado (ex.: botao web_app
    // invalido), a mensagem some sem erro. Aqui logamos o motivo exato.
    JsonDocument resp, filter;
    filter["ok"] = true;
    filter["description"] = true;
    bool http = tgPostJson("sendMessage", body, &resp, &filter);
    bool ok   = resp["ok"] | false;
    if (!ok)
        Serial.printf("[tg] teclado recusado: %s\n",
                      (const char *)(resp["description"] | "(sem resposta)"));
    return http && ok;
}

// ------------------------------------------------------------------- foto
#if HAS_CAMERA
static bool tgSendPhoto(const String &caption) {
    if (!tgEnabled() || !strlen(cfg.tgChat) || !cameraReady()) return false;
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) return false;

    WiFiClientSecure cli;
    if (!tgConnect(cli, 15000)) { esp_camera_fb_return(fb); return false; }

    const char *B = "----aquafeeder";
    String p1, p2, tail;
    p1  = "--"; p1 += B; p1 += "\r\nContent-Disposition: form-data; name=\"chat_id\"\r\n\r\n";
    p1 += cfg.tgChat; p1 += "\r\n";
    p1 += "--"; p1 += B; p1 += "\r\nContent-Disposition: form-data; name=\"caption\"\r\n\r\n";
    p1 += caption; p1 += "\r\n";
    p2  = "--"; p2 += B;
    p2 += "\r\nContent-Disposition: form-data; name=\"photo\"; filename=\"aquario.jpg\"\r\n";
    p2 += "Content-Type: image/jpeg\r\n\r\n";
    tail = "\r\n--"; tail += B; tail += "--\r\n";

    size_t total = p1.length() + p2.length() + fb->len + tail.length();
    String head;
    head.reserve(220);
    head  = "POST /bot"; head += cfg.tgToken; head += "/sendPhoto HTTP/1.1\r\n";
    head += "Host: "; head += TG_HOST; head += "\r\n";
    head += "Content-Type: multipart/form-data; boundary="; head += B; head += "\r\n";
    head += "Content-Length: "; head += total; head += "\r\n";
    head += "Connection: close\r\n\r\n";

    cli.print(head);
    cli.print(p1);
    cli.print(p2);
    // envia o JPEG em blocos para nao estourar o buffer do TLS
    const size_t CH = 2048;
    for (size_t off = 0; off < fb->len; off += CH) {
        size_t n = (fb->len - off) > CH ? CH : (fb->len - off);
        if (cli.write(fb->buf + off, n) != n) break;
    }
    cli.print(tail);
    esp_camera_fb_return(fb);

    skipHeaders(cli, 15000);
    cli.stop();
    return true;
}
#endif

void tgNotifyFeed(uint8_t portions, const char *src, bool ok, uint16_t pulses) {
    if (!tgEnabled() || !cfg.tgNotify) return;
    String m = ok ? "🐟 Alimentei " : "⚠️ Tentei alimentar ";
    m += portions;
    m += portions == 1 ? " porção" : " porções";
    m += " (";
    m += src;
    m += ")";
    if (cfg.sensorEnabled) { m += ", grãos: "; m += pulses; }
    if (!ok) m += "\nO sensor não detectou ração! Pode estar entupido ou vazio.";
    tgSend(m);
}

// ---------------------------------------------------------------- comandos
static String statusText() {
    char w[24];
    fmtWhen(feederLastFeed(), w, sizeof(w));
    time_t now = time(nullptr);
    struct tm tmv;
    localtime_r(&now, &tmv);
    char hora[16];
    strftime(hora, sizeof(hora), "%H:%M", &tmv);

    String s = "🐠 AquaFeeder\n";
    s += "Hora do aparelho: "; s += hora; s += "\n";
    s += "Porções hoje: ";  s += feederFedToday(); s += "/"; s += cfg.maxPerDay; s += "\n";
    s += "Última: ";        s += w; s += "\n";
    s += "Wi-Fi: ";         s += WiFi.RSSI(); s += " dBm\n";
    s += "Total desde sempre: "; s += feederTotalPortions(); s += " porções";
    return s;
}

static String agendaText() {
    const char *dn[7] = {"dom", "seg", "ter", "qua", "qui", "sex", "sab"};
    String s = "⏰ Agenda\n";
    bool any = false;
    for (int i = 0; i < MAX_SLOTS; i++) {
        if (!cfg.slots[i].en) continue;
        any = true;
        char buf[16];
        snprintf(buf, sizeof(buf), "%02u:%02u", cfg.slots[i].h, cfg.slots[i].m);
        s += buf;
        s += "  ";
        s += cfg.slots[i].portions;
        s += "x  ";
        if ((cfg.slots[i].dow & 0x7F) == 0x7F) s += "todos os dias";
        else for (int d = 0; d < 7; d++)
            if (cfg.slots[i].dow & (1 << d)) { s += dn[d]; s += " "; }
        s += "\n";
    }
    if (!any) s += "(nenhum horário ativo)";
    return s;
}

static void handleCommand(const String &chat, String text) {
    // trava: so o chat configurado pode dar ordens
    if (!strlen(cfg.tgChat)) {
        JsonDocument d;
        d["chat_id"] = chat;
        d["text"]    = String("Seu chat id é ") + chat +
                       ". Coloque esse número em Ajustes > Telegram para liberar os comandos.";
        String body;
        serializeJson(d, body);
        tgPostJson("sendMessage", body);
        Serial.printf("[tg] chat nao autorizado: %s\n", chat.c_str());
        return;
    }
    if (chat != cfg.tgChat) {
        Serial.printf("[tg] ignorando chat %s\n", chat.c_str());
        return;
    }

    text.trim();

    // O painel (Mini App) manda os ajustes como JSON. Tratar ANTES do
    // toLowerCase, que corromperia as chaves/valores do JSON.
    if (text.startsWith("{") && text.endsWith("}")) {
        if (text.length() > 3500) { tgSend("⚠️ Configuração grande demais."); return; }
        JsonDocument d;
        DeserializationError e = deserializeJson(d, text);
        if (!e) {
            settingsFromJson(d.as<JsonObjectConst>());
            settingsSave();
            cameraApplySettings();
            feederApplySensor();
            setenv("TZ", cfg.tz, 1);
            tzset();
            // reenvia o teclado: confirma e ja atualiza o link com o novo retrato
            tgSendKb("✅ Configuração salva pelo painel.");
        } else {
            tgSend(String("⚠️ Não consegui ler a configuração: ") + e.c_str());
        }
        return;
    }

    text.toLowerCase();

    // Os botoes do teclado mandam um rotulo amigavel (ex.: "🐟 Alimentar 2x").
    // Se nao for um /comando digitado, traduz o rotulo para o comando.
    if (!text.startsWith("/")) {
        if (text.indexOf("alimentar") >= 0) {
            String num;
            for (unsigned int i = 0; i < text.length(); i++)
                if (isDigit(text[i])) num += text[i];
            text = num.length() ? ("/alimentar " + num) : "/alimentar";
        } else if (text.indexOf("foto")   >= 0) text = "/foto";
        else if (text.indexOf("status")   >= 0) text = "/status";
        else if (text.indexOf("agenda")   >= 0) text = "/agenda";
        else if (text.indexOf("menu")     >= 0) text = "/menu";
    }

    int sp = text.indexOf(' ');
    String cmd = sp > 0 ? text.substring(0, sp) : text;
    String arg = sp > 0 ? text.substring(sp + 1) : "";
    if (cmd.indexOf('@') > 0) cmd = cmd.substring(0, cmd.indexOf('@'));

    if (cmd == "/alimentar" || cmd == "/feed") {
        int n = arg.length() ? arg.toInt() : 1;
        if (n < 1) n = 1;
        String err;
        if (feederRequest((uint8_t)n, SRC_TELEGRAM, err))
            tgSend(String("Ok, soltando ") + n + (n == 1 ? " porção..." : " porções..."));
        else
            tgSend(String("Não deu: ") + err);
    } else if (cmd == "/foto" || cmd == "/photo") {
#if HAS_CAMERA
        if (!cameraReady()) tgSend("Câmera indisponível.");
        else if (!tgSendPhoto("📷 aquário agora")) tgSend("Falhou ao enviar a foto.");
#else
        tgSend("Este firmware foi compilado sem câmera.");
#endif
    } else if (cmd == "/status") {
        tgSend(statusText());
    } else if (cmd == "/jog") {
        int32_t s = arg.length() ? arg.toInt() : (STEPS_PER_REV / 4);
        feederJog(s);
        tgSend(String("Movendo a rosca (") + s + " passos)...");
    } else if (cmd == "/agenda") {
        tgSend(agendaText());
    } else if (cmd == "/reiniciar" || cmd == "/reboot") {
        tgSend("Reiniciando o aparelho...");
        delay(400);
        ESP.restart();
    } else if (cmd == "/start" || cmd == "/help" || cmd == "/ajuda" || cmd == "/menu") {
        tgSendKb("🐠 AquaFeeder\nToque nos botões abaixo — ou use os comandos:\n"
                 "/alimentar [n] · /foto · /status · /agenda");
    }
}

// ----------------------------------------------------------------- polling
static void pollOnce() {
    JsonDocument filter;
    filter["result"][0]["update_id"] = true;
    filter["result"][0]["message"]["text"] = true;
    filter["result"][0]["message"]["chat"]["id"] = true;
    filter["result"][0]["message"]["web_app_data"]["data"] = true;   // painel (Mini App)

    JsonDocument body;
    body["timeout"] = drained ? 20 : 0;
    body["limit"]   = 5;
    if (drained) body["offset"] = tgOffset;
    else         body["offset"] = -1;      // 1a vez: descarta fila antiga
    JsonArray au = body["allowed_updates"].to<JsonArray>();
    au.add("message");

    String req;
    serializeJson(body, req);

    JsonDocument resp;
    if (!tgPostJson("getUpdates", req, &resp, &filter, drained ? 32000 : 12000)) return;

    JsonArrayConst results = resp["result"];
    for (JsonObjectConst up : results) {
        int64_t id = up["update_id"] | 0;
        if (id >= tgOffset) tgOffset = id + 1;
        if (!drained) continue;                 // descarta o backlog do boot
        JsonObjectConst msg = up["message"];
        if (msg.isNull()) continue;
        // Comando pode vir como texto (digitado/botao) ou como web_app_data (painel)
        const char *wad = msg["web_app_data"]["data"] | "";
        const char *txt = msg["text"] | "";
        const char *payload = wad[0] ? wad : txt;
        int64_t chatId  = msg["chat"]["id"] | 0;
        if (!payload[0] || chatId == 0) continue;
        char chatStr[24];
        snprintf(chatStr, sizeof(chatStr), "%lld", (long long)chatId);
        Serial.printf("[tg] %s: %s%s\n", chatStr, payload, wad[0] ? " (painel)" : "");
        handleCommand(String(chatStr), String(payload));
    }
    if (!drained) {
        drained = true;
        Serial.printf("[tg] pronto, offset inicial %lld\n", (long long)tgOffset);
    }
}

static void tgTask(void *) {
    vTaskDelay(pdMS_TO_TICKS(6000));
    for (;;) {
        if (tgEnabled() && WiFi.status() == WL_CONNECTED) {
            pollOnce();
            vTaskDelay(pdMS_TO_TICKS(500));
        } else {
            vTaskDelay(pdMS_TO_TICKS(5000));
        }
    }
}

void tgBegin() {
    if (!tgEnabled()) { Serial.println(F("[tg] desligado (sem token)")); return; }
    xTaskCreatePinnedToCore(tgTask, "telegram", 12288, nullptr, 1, nullptr, 0);
    Serial.println(F("[tg] task iniciada"));
}
