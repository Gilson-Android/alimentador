// =====================================================================
//  AquaFeeder -- controle do motor 28BYJ-48 / ULN2003 e travas de seguranca
// =====================================================================
#pragma once
#include <Arduino.h>
#include "config.h"

enum FeedSrc : uint8_t {
    SRC_MANUAL = 0,
    SRC_SCHED,
    SRC_TELEGRAM,
    SRC_BUTTON,
    SRC_JOG,
    SRC_ALEXA
};

struct FeedEntry {
    time_t   ts;
    uint8_t  portions;
    uint8_t  src;
    uint16_t pulses;    // grãos detectados (0 se sensor desligado)
    bool     ok;
};

// avisado ao terminar cada alimentacao (usado pelo Telegram)
typedef void (*FeedDoneCb)(uint8_t portions, const char *src, bool ok, uint16_t pulses);
void        feederOnDone(FeedDoneCb cb);

void        feederInit();
// Enfileira uma alimentacao. Retorna false e preenche err se alguma trava barrar.
bool        feederRequest(uint8_t portions, FeedSrc src, String &err);
// Giro manual bruto, para desentupir / calibrar (+ = frente, - = re)
void        feederJog(int32_t halfSteps);
bool        feederBusy();
uint8_t     feederFedToday();
time_t      feederLastFeed();
uint32_t    feederTotalPortions();
size_t      feederLogCopy(FeedEntry *dst, size_t max);
const char *feedSrcName(uint8_t src);
