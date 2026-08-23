// =====================================================================
//  AquaFeeder -- MQTT (comando pela nuvem, sem abrir porta no roteador)
//
//  Quem inicia a conexao e SEMPRE o ESP32: ele sai para o broker e deixa a
//  conexao pendurada, assinando <prefixo>/cmd. Quando alguem publica nesse
//  topico, o comando chega em milissegundos.
//
//  Topicos (prefixo configuravel, padrao "aquafeeder"):
//    aquafeeder/online   "1" / "0"   retido -- "0" e publicado pelo BROKER
//                                    se o aparelho cair (Last Will)
//    aquafeeder/status   JSON        retido, a cada 60 s e a cada evento
//    aquafeeder/evento   JSON        uma mensagem por alimentacao
//    aquafeeder/cmd      comando     voce publica aqui
//
//  O "online" com Last Will e o motivo principal de usar MQTT em viagem:
//  se o alimentador travar, ficar sem luz ou sem internet, o proprio broker
//  avisa. Com Telegram voce so recebe silencio, que e indistinguivel de
//  "esta tudo bem e ninguem mandou mensagem".
// =====================================================================
#pragma once
#include <Arduino.h>

void mqttBegin();
bool mqttEnabled();
bool mqttOnline();
void mqttNotifyFeed(uint8_t portions, const char *src, bool ok, uint16_t pulses);
