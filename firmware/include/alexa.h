// =====================================================================
//  AquaFeeder -- Alexa via Sinric Pro (nuvem gratuita, sem abrir porta)
//
//  Mesmo padrao do Telegram/MQTT: quem conecta e SEMPRE o ESP32, para fora,
//  por um WebSocket seguro. A Alexa manda o comando para a nuvem do Sinric,
//  que o repassa ao aparelho em milissegundos -- funciona em qualquer Echo.
//
//  IMPORTANTE sobre a frase: a Alexa nao aceita um comando cru "alimentar
//  agora". Qualquer aparelho de casa inteligente responde a
//  "Alexa, ligar alimentador". Para dizer "Alexa, ALIMENTAR AGORA", crie uma
//  ROTINA no app Alexa:  gatilho de voz "alimentar agora"  ->  acao "Ligar"
//  neste dispositivo.
//
//  Configuracao (uma vez):
//    1. Crie conta gratis em https://portal.sinric.pro
//    2. Devices > Add Device: tipo "Switch", nome "alimentador".
//    3. Anote o Device ID (do dispositivo) e, em Credentials, o App Key e o
//       App Secret. Cole os tres em Ajustes do painel (alexaAppKey/Secret/DevId).
//    4. No app Alexa: Dispositivos > + > Skill "Sinric Pro" (vincular conta),
//       depois "procurar dispositivos" ate achar o "alimentador".
//    5. Rotinas > + > "Quando eu disser" = alimentar agora  ->  Ligar alimentador.
// =====================================================================
#pragma once
#include <Arduino.h>

void alexaBegin();     // sobe a task do Sinric Pro (so se as chaves existirem)
bool alexaEnabled();
