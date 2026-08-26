#include "feeder.h"
#include "settings.h"
#include <Preferences.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>

// --------------------------------------------------------------------- estado
#if MOTOR_DRIVER == DRV_ULN2003
static const uint8_t PINS[4] = {PIN_IN1, PIN_IN2, PIN_IN3, PIN_IN4};

// sequencia de meio-passo do 28BYJ-48
static const uint8_t SEQ[8][4] = {
    {1,0,0,0}, {1,1,0,0}, {0,1,0,0}, {0,1,1,0},
    {0,0,1,0}, {0,0,1,1}, {0,0,0,1}, {1,0,0,1}
};
#endif

// src == SRC_JOG -> executa "steps" cru; caso contrario dispensa "portions"
struct Request { int32_t steps; uint8_t portions; uint8_t src; };

static QueueHandle_t   q          = nullptr;
static SemaphoreHandle_t mtx      = nullptr;
static volatile bool   busy       = false;
static int8_t          seqIdx     = 0;

static FeedEntry logBuf[LOG_SIZE];
static uint8_t   logHead = 0, logCount = 0;

static Preferences   nvs;
static uint8_t       fedToday   = 0;
static uint32_t      dayKey     = 0;
static time_t        lastFeed   = 0;
static uint32_t      totalPort  = 0;
static uint32_t      lastReqMs  = 0;

static volatile uint32_t pulses      = 0;
static volatile uint32_t lastPulseUs = 0;
static FeedDoneCb        doneCb      = nullptr;

void feederOnDone(FeedDoneCb cb) { doneCb = cb; }

// --------------------------------------------------------------- utilitarios
static void IRAM_ATTR onPulse() {
    uint32_t t = micros();
    if (t - lastPulseUs > 1500) { pulses++; lastPulseUs = t; }
}

static uint32_t todayKey() {
    time_t now = time(nullptr);
    struct tm tmv;
    localtime_r(&now, &tmv);
    if (tmv.tm_year + 1900 < 2024) return 0;   // hora ainda nao sincronizada
    return (uint32_t)(tmv.tm_year + 1900) * 10000 + (tmv.tm_mon + 1) * 100 + tmv.tm_mday;
}

static void statSave() {
    nvs.begin("feedstat", false);
    nvs.putUChar("fed", fedToday);
    nvs.putULong("day", dayKey);
    nvs.putULong64("last", (uint64_t)lastFeed);
    nvs.putULong("total", totalPort);
    nvs.end();
}

static void statLoad() {
    nvs.begin("feedstat", true);
    fedToday  = nvs.getUChar("fed", 0);
    dayKey    = nvs.getULong("day", 0);
    lastFeed  = (time_t)nvs.getULong64("last", 0);
    totalPort = nvs.getULong("total", 0);
    nvs.end();
}

static void rollDay() {
    uint32_t k = todayKey();
    if (k && k != dayKey) { dayKey = k; fedToday = 0; statSave(); }
}

// espera entre passos. delayMicroseconds nao cede CPU, entao fatiamos
static inline void stepDelay(uint32_t us) {
    if (us > 2000) {
        delay(us / 1000);
        delayMicroseconds(us % 1000);
    } else {
        delayMicroseconds(us);
    }
}

#if MOTOR_DRIVER == DRV_ULN2003
// ------------------------------------------------- 28BYJ-48 via ULN2003
static void coilsOff() {
    for (int i = 0; i < 4; i++) digitalWrite(PINS[i], LOW);
}

static void writeStep() {
    for (int i = 0; i < 4; i++) digitalWrite(PINS[i], SEQ[seqIdx][i]);
}

// steps > 0 = sentido de dispensa (ja considerando cfg.reverse)
static void runSteps(int32_t steps) {
    int dir = (steps >= 0) ? 1 : -1;
    if (cfg.reverse) dir = -dir;
    uint32_t n = abs(steps);
    for (uint32_t i = 0; i < n; i++) {
        seqIdx = (seqIdx + dir + 8) % 8;
        writeStep();
        stepDelay(cfg.stepUs);
        if ((i & 0x3F) == 0x3F) vTaskDelay(1);   // ~a cada 64 passos
    }
    coilsOff();   // solta as bobinas: nao esquenta parado
}

#else
// --------------------------------- NEMA 17 via A4988 / DRV8825 / TMC2208
static void driverEnable(bool on) {
    digitalWrite(PIN_EN, on ? LOW : HIGH);   // ENABLE ativo em nivel baixo
}

static void runSteps(int32_t steps) {
    int dir = (steps >= 0) ? 1 : -1;
    if (cfg.reverse) dir = -dir;
    digitalWrite(PIN_DIR, dir > 0 ? HIGH : LOW);
    driverEnable(true);
    delayMicroseconds(200);              // driver acordar + DIR estabilizar

    uint32_t n = abs(steps);
    // Perfil trapezoidal: parte lento (start), acelera ate a velocidade de
    // cruzeiro (cfg.stepUs) em 'ramp' passos, e desacelera no fim. Isso vence
    // a inercia do NEMA17 sem travar no arranque.
    uint32_t cruise = cfg.stepUs;
    uint32_t start  = (RAMP_START_US > cruise) ? RAMP_START_US : cruise;
    uint32_t ramp   = RAMP_STEPS;
    if (ramp > n / 2) ramp = n / 2;     // move curto: rampa cabe na metade

    for (uint32_t i = 0; i < n; i++) {
        uint32_t per = cruise;          // periodo (us) deste passo
        if (ramp > 0) {
            if (i < ramp)                                  // acelerando
                per = start - (uint32_t)((uint64_t)(start - cruise) * i / ramp);
            else if (i >= n - ramp)                        // desacelerando
                per = start - (uint32_t)((uint64_t)(start - cruise) * (n - 1 - i) / ramp);
        }
        digitalWrite(PIN_STEP, HIGH);
        delayMicroseconds(4);            // pulso minimo (DRV8825 pede >=1.9us)
        digitalWrite(PIN_STEP, LOW);
        stepDelay(per > 6 ? per - 4 : 2);
        if ((i & 0x3F) == 0x3F) vTaskDelay(1);
    }
    driverEnable(false);   // solta o motor: nao esquenta nem consome parado
}
#endif

static void logPush(uint8_t portions, uint8_t src, uint16_t p, bool ok) {
    logBuf[logHead] = {time(nullptr), portions, src, p, ok};
    logHead = (logHead + 1) % LOG_SIZE;
    if (logCount < LOG_SIZE) logCount++;
}

static void ledSet(bool on) {
#if PIN_STATUS_LED >= 0
    digitalWrite(PIN_STATUS_LED, STATUS_LED_INVERT ? !on : on);
#else
    (void)on;
#endif
}

// ------------------------------------------------------------------- execucao
static void dispense(uint8_t portions, uint8_t src) {
    busy = true;
    ledSet(true);
    int32_t steps = (int32_t)portions * cfg.stepsPerPortion;

    // 1) sacode para tras: quebra "ponte" de racao na boca do funil
    runSteps(-ANTIJAM_STEPS);
    delay(120);

    // 2) dispensa contando graos (se houver sensor)
    pulses = 0;
    runSteps(steps);
    delay(150);
    uint16_t p1 = pulses;

    bool ok = true;
    if (cfg.sensorEnabled && p1 == 0) {
        // nada caiu: tenta desentupir com um recuo maior e repete meia dose
        Serial.println(F("[feeder] nada detectado, tentando desentupir"));
        runSteps(-UNJAM_STEPS);
        delay(200);
        pulses = 0;
        runSteps(steps);
        delay(150);
        if (pulses == 0) ok = false;
        p1 += pulses;
    }

    lastFeed = time(nullptr);
    totalPort += portions;
    logPush(portions, src, p1, ok);
    statSave();
    ledSet(false);
    busy = false;

    Serial.printf("[feeder] %u porcao(oes) via %s, graos=%u, ok=%d\n",
                  portions, feedSrcName(src), p1, ok);
    if (doneCb) doneCb(portions, feedSrcName(src), ok, p1);
}

static void feederTask(void *) {
    Request r;
    for (;;) {
        if (xQueueReceive(q, &r, portMAX_DELAY) == pdTRUE) {
            if (r.src == SRC_JOG) {
                busy = true;
                ledSet(true);
                runSteps(r.steps);
                ledSet(false);
                busy = false;
            } else {
                dispense(r.portions, r.src);
            }
        }
    }
}

// ---------------------------------------------------------------------- API
void feederInit() {
#if MOTOR_DRIVER == DRV_ULN2003
    for (int i = 0; i < 4; i++) { pinMode(PINS[i], OUTPUT); digitalWrite(PINS[i], LOW); }
#else
    pinMode(PIN_STEP, OUTPUT); digitalWrite(PIN_STEP, LOW);
    pinMode(PIN_DIR,  OUTPUT); digitalWrite(PIN_DIR, LOW);
    pinMode(PIN_EN,   OUTPUT); driverEnable(false);
#endif
#if PIN_STATUS_LED >= 0
    pinMode(PIN_STATUS_LED, OUTPUT);
    ledSet(false);
#endif
    if (cfg.sensorEnabled && PIN_FEED_SENSOR >= 0) {
        pinMode(PIN_FEED_SENSOR, INPUT_PULLUP);
        attachInterrupt(digitalPinToInterrupt(PIN_FEED_SENSOR), onPulse, FALLING);
    }
    statLoad();
    rollDay();
    mtx = xSemaphoreCreateMutex();
    q   = xQueueCreate(6, sizeof(Request));
    xTaskCreatePinnedToCore(feederTask, "feeder", 4096, nullptr, 3, nullptr, 1);
    Serial.printf("[feeder] pronto. %u passos/porcao, %u us/passo, hoje=%u\n",
                  cfg.stepsPerPortion, cfg.stepUs, fedToday);
}

bool feederRequest(uint8_t portions, FeedSrc src, String &err) {
    rollDay();
    if (portions == 0) portions = 1;

    if (xSemaphoreTake(mtx, pdMS_TO_TICKS(500)) != pdTRUE) {
        err = F("ocupado");
        return false;
    }
    bool okToGo = false;
    do {
        if (busy || uxQueueMessagesWaiting(q) > 0) { err = F("Já existe uma alimentação em andamento."); break; }
        if (portions > cfg.maxPerRequest) {
            err = String(F("Máximo por vez: ")) + cfg.maxPerRequest + F(" porções.");
            break;
        }
        if (fedToday + portions > cfg.maxPerDay) {
            err = String(F("Limite diário atingido (")) + cfg.maxPerDay + F(" porções). Trava de segurança.");
            break;
        }
        uint32_t nowMs = millis();
        if (src != SRC_SCHED && lastReqMs && (nowMs - lastReqMs) < (uint32_t)cfg.minIntervalS * 1000UL) {
            uint32_t falta = (cfg.minIntervalS * 1000UL - (nowMs - lastReqMs)) / 1000UL + 1;
            err = String(F("Aguarde ")) + falta + F("s (intervalo mínimo).");
            break;
        }
        okToGo = true;
    } while (0);

    if (okToGo) {
        fedToday += portions;
        lastReqMs = millis();
    }
    xSemaphoreGive(mtx);
    if (!okToGo) return false;

    Request r{0, portions, (uint8_t)src};
    if (xQueueSend(q, &r, 0) != pdTRUE) {
        err = F("fila cheia");
        return false;
    }
    return true;
}

void feederJog(int32_t halfSteps) {
    if (busy || uxQueueMessagesWaiting(q) > 0) return;
    halfSteps = constrain(halfSteps, -4 * STEPS_PER_REV, 4 * STEPS_PER_REV);
    Request r{halfSteps, 0, (uint8_t)SRC_JOG};
    xQueueSend(q, &r, 0);
}

bool        feederBusy()          { return busy; }
uint8_t     feederFedToday()      { rollDay(); return fedToday; }
time_t      feederLastFeed()      { return lastFeed; }
uint32_t    feederTotalPortions() { return totalPort; }

size_t feederLogCopy(FeedEntry *dst, size_t max) {
    size_t n = min((size_t)logCount, max);
    for (size_t i = 0; i < n; i++) {
        // do mais novo para o mais antigo
        int idx = (logHead - 1 - (int)i + 2 * LOG_SIZE) % LOG_SIZE;
        dst[i] = logBuf[idx];
    }
    return n;
}

const char *feedSrcName(uint8_t src) {
    switch (src) {
        case SRC_MANUAL:   return "manual";
        case SRC_SCHED:    return "agenda";
        case SRC_TELEGRAM: return "telegram";
        case SRC_BUTTON:   return "botao";
        default:           return "jog";
    }
}
