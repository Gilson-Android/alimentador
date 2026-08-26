// =====================================================================
//  AquaFeeder -- pinos, limites e valores padrao
// =====================================================================
#pragma once
#include <Arduino.h>

#define FW_VERSION "1.0.0"

// --------------------------------------------------------------- placa
#define BOARD_FREENOVE_S3_CAM 1
#define BOARD_XIAO_S3_SENSE   2
#define BOARD_S3_NO_CAMERA    3

#ifndef FEEDER_BOARD
  #define FEEDER_BOARD BOARD_FREENOVE_S3_CAM
#endif

#if FEEDER_BOARD == BOARD_FREENOVE_S3_CAM
  #define HAS_CAMERA        1
  #define PIN_IN1           1     // ULN2003 IN1
  #define PIN_IN2           2     // ULN2003 IN2
  #define PIN_IN3           41    // ULN2003 IN3
  #define PIN_IN4           42    // ULN2003 IN4
  #define PIN_BUTTON        0     // botao BOOT da placa (alimenta 1 porcao)
  #define PIN_FEED_SENSOR   21    // opcional: sensor optico na calha
  #define PIN_STATUS_LED    -1
#elif FEEDER_BOARD == BOARD_XIAO_S3_SENSE
  #define HAS_CAMERA        1
  #define PIN_IN1           1     // D0
  #define PIN_IN2           2     // D1
  #define PIN_IN3           3     // D2
  #define PIN_IN4           4     // D3
  #define PIN_BUTTON        -1    // XIAO nao expoe o BOOT
  #define PIN_FEED_SENSOR   5     // D4
  #define PIN_STATUS_LED    21    // LED interno (invertido)
  #define STATUS_LED_INVERT 1
#else
  #define HAS_CAMERA        0
  #define PIN_IN1           1
  #define PIN_IN2           2
  #define PIN_IN3           41
  #define PIN_IN4           42
  #define PIN_BUTTON        0
  #define PIN_FEED_SENSOR   21
  #define PIN_STATUS_LED    -1
#endif

#ifndef STATUS_LED_INVERT
  #define STATUS_LED_INVERT 0
#endif

// Driver STEP/DIR usa 3 dos 4 GPIOs do motor
#define PIN_STEP  PIN_IN1
#define PIN_DIR   PIN_IN2
#define PIN_EN    PIN_IN3   // ENABLE do A4988/DRV8825 e ativo em nivel BAIXO

// -------------------------------------------------------------- driver
// DRV_ULN2003: motor 28BYJ-48 de 5V com redutor, driver ULN2003 (4 fios)
// DRV_STEPDIR: NEMA 17 com A4988 / DRV8825 / TMC2208 (STEP-DIR-EN) + 12V
#define DRV_ULN2003 1
#define DRV_STEPDIR 2
#ifndef MOTOR_DRIVER
  #define MOTOR_DRIVER DRV_ULN2003
#endif

// Frequencia do clock da camera. 20 MHz e o padrao. Se voce ligou o OV2640
// na mao, com jumpers, e a imagem sai listrada/verde, caia para 10000000 --
// fio comprido em barramento paralelo de 20 MHz e a causa numero 1 disso.
#ifndef CAM_XCLK_HZ
  #define CAM_XCLK_HZ 20000000
#endif

// ------------------------------------------------------------ mecanica
#if MOTOR_DRIVER == DRV_STEPDIR
  // NEMA 17: 200 passos/volta. Deixe o driver em 1/8 de micropasso
  // (A4988: MS1=H MS2=H | DRV8825: M0=H M1=H | TMC2208: 1/8 por padrao)
  #define MICROSTEPS             8
  #define STEPS_PER_REV          (200 * MICROSTEPS)   // 1600
  #define DEF_STEPS_PER_PORTION  (STEPS_PER_REV / 4)  // 400 = 1/4 volta ~= 0,23 ml
  #define DEF_STEP_US            900    // us entre passos
  #define ANTIJAM_STEPS          36     // re antes de dispensar (quebra ponte)
  #define UNJAM_STEPS            160    // re maior quando detecta entupimento
#else
  // 28BYJ-48 em meio-passo: 4096 passos por volta do eixo de saida
  #define STEPS_PER_REV          4096
  #define DEF_STEPS_PER_PORTION  1024   // 1/4 de volta ~= 0,23 ml
  #define DEF_STEP_US            1300   // us por meio-passo (nao abaixe de 1000)
  #define ANTIJAM_STEPS          90
  #define UNJAM_STEPS            400
#endif

// -------------------------------------------------------------- limites
#define MAX_SLOTS                8
#define LOG_SIZE                 24
#define DEF_MAX_PER_DAY          10     // trava de seguranca (nao matar o peixe)
#define DEF_MAX_PER_REQUEST      4
#define DEF_MIN_INTERVAL_S       45
#define DEF_CATCHUP_MIN          90     // recupera horario perdido (queda de luz)

// ----------------------------------------------------------------- rede
#define AP_SSID_PREFIX  "AquaFeeder-"
#define AP_PASSWORD     "aquafeeder"   // senha do modo configuracao
#define HTTP_PORT       80
#define STREAM_PORT     81
#define WIFI_TIMEOUT_MS 25000

#define DEF_HOSTNAME    "aquafeeder"
#define DEF_TZ          "<-03>3"       // Brasil (sem horario de verao)
#define DEF_NTP1        "pool.ntp.org"
#define DEF_NTP2        "time.google.com"

// ----------------------------------------------- painel (Telegram Mini App)
// Cole aqui a URL HTTPS onde voce hospedou a pasta painel/ (GitHub Pages,
// Netlify, Cloudflare Pages...). Ex.: "https://usuario.github.io/Alimentador/painel/"
// Vazio = sem o botao "Abrir painel" (o teclado mostra so os botoes de texto).
// TEM que ser https -- o Telegram recusa http.
#ifndef TG_WEBAPP_URL
  #define TG_WEBAPP_URL ""
#endif
