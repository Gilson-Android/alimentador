// =====================================================================
//  AquaFeeder -- servidor HTTP (interface + API + stream MJPEG)
// =====================================================================
#pragma once
#include <Arduino.h>

void webStart();
void webStop();
bool webRebootRequested();
// formata epoch como "23/08 14:35" ("-" se hora nao sincronizada)
void fmtWhen(time_t t, char *out, size_t n);
