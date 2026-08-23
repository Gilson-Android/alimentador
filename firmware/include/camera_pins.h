// =====================================================================
//  AquaFeeder -- pinagem do modulo de camera por placa
// =====================================================================
#pragma once
#include "config.h"

#if FEEDER_BOARD == BOARD_FREENOVE_S3_CAM
  // Freenove ESP32-S3-WROOM CAM
  #define PWDN_GPIO_NUM   -1
  #define RESET_GPIO_NUM  -1
  #define XCLK_GPIO_NUM   15
  #define SIOD_GPIO_NUM    4
  #define SIOC_GPIO_NUM    5
  #define Y9_GPIO_NUM     16
  #define Y8_GPIO_NUM     17
  #define Y7_GPIO_NUM     18
  #define Y6_GPIO_NUM     12
  #define Y5_GPIO_NUM     10
  #define Y4_GPIO_NUM      8
  #define Y3_GPIO_NUM      9
  #define Y2_GPIO_NUM     11
  #define VSYNC_GPIO_NUM   6
  #define HREF_GPIO_NUM    7
  #define PCLK_GPIO_NUM   13

#elif FEEDER_BOARD == BOARD_XIAO_S3_SENSE
  // Seeed XIAO ESP32S3 Sense
  #define PWDN_GPIO_NUM   -1
  #define RESET_GPIO_NUM  -1
  #define XCLK_GPIO_NUM   10
  #define SIOD_GPIO_NUM   40
  #define SIOC_GPIO_NUM   39
  #define Y9_GPIO_NUM     48
  #define Y8_GPIO_NUM     11
  #define Y7_GPIO_NUM     12
  #define Y6_GPIO_NUM     14
  #define Y5_GPIO_NUM     16
  #define Y4_GPIO_NUM     18
  #define Y3_GPIO_NUM     17
  #define Y2_GPIO_NUM     15
  #define VSYNC_GPIO_NUM  38
  #define HREF_GPIO_NUM   47
  #define PCLK_GPIO_NUM   13
#endif
