// =====================================================================
//  AquaFeeder -- bot do Telegram (controle de fora de casa, sem abrir
//  porta no roteador). Implementado direto sobre HTTPS, sem bibliotecas.
// =====================================================================
#pragma once
#include <Arduino.h>

void tgBegin();                                  // sobe a task de polling
void tgNotifyFeed(uint8_t portions, const char *src, bool ok, uint16_t pulses);
bool tgEnabled();
