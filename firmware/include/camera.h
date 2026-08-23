// =====================================================================
//  AquaFeeder -- camera (OV2640)
// =====================================================================
#pragma once
#include <Arduino.h>
#include "config.h"

bool cameraInit();
bool cameraReady();
void cameraApplySettings();   // aplica flip/espelho/qualidade/tamanho da cfg
const char *cameraLastError();
