#include "camera.h"
#include "settings.h"

#if HAS_CAMERA
#include "esp_camera.h"
#include "camera_pins.h"

static bool ready = false;
static char lastErr[48] = "";

static framesize_t sizeOf(uint8_t s) {
    switch (s) {
        case 0:  return FRAMESIZE_QVGA;   // 320x240
        case 1:  return FRAMESIZE_VGA;    // 640x480
        case 2:  return FRAMESIZE_SVGA;   // 800x600
        default: return FRAMESIZE_HD;     // 1280x720
    }
}

bool cameraInit() {
    camera_config_t c = {};
    c.ledc_channel = LEDC_CHANNEL_0;
    c.ledc_timer   = LEDC_TIMER_0;
    c.pin_d0 = Y2_GPIO_NUM;   c.pin_d1 = Y3_GPIO_NUM;
    c.pin_d2 = Y4_GPIO_NUM;   c.pin_d3 = Y5_GPIO_NUM;
    c.pin_d4 = Y6_GPIO_NUM;   c.pin_d5 = Y7_GPIO_NUM;
    c.pin_d6 = Y8_GPIO_NUM;   c.pin_d7 = Y9_GPIO_NUM;
    c.pin_xclk = XCLK_GPIO_NUM;
    c.pin_pclk = PCLK_GPIO_NUM;
    c.pin_vsync = VSYNC_GPIO_NUM;
    c.pin_href  = HREF_GPIO_NUM;
    c.pin_sccb_sda = SIOD_GPIO_NUM;
    c.pin_sccb_scl = SIOC_GPIO_NUM;
    c.pin_pwdn  = PWDN_GPIO_NUM;
    c.pin_reset = RESET_GPIO_NUM;
    c.xclk_freq_hz = 20000000;
    c.pixel_format = PIXFORMAT_JPEG;
    c.grab_mode    = CAMERA_GRAB_LATEST;

    bool psram = psramFound();
    c.frame_size   = psram ? sizeOf(cfg.camSize) : FRAMESIZE_VGA;
    c.jpeg_quality = psram ? cfg.camQuality : 16;
    c.fb_count     = psram ? 2 : 1;
    c.fb_location  = psram ? CAMERA_FB_IN_PSRAM : CAMERA_FB_IN_DRAM;

    esp_err_t e = esp_camera_init(&c);
    if (e != ESP_OK) {
        snprintf(lastErr, sizeof(lastErr), "esp_camera_init 0x%x", e);
        Serial.printf("[cam] FALHOU: %s\n", lastErr);
        ready = false;
        return false;
    }
    ready = true;
    cameraApplySettings();
    Serial.printf("[cam] ok (psram=%d, tam=%u, q=%u)\n", psram, cfg.camSize, cfg.camQuality);
    return true;
}

void cameraApplySettings() {
    if (!ready) return;
    sensor_t *s = esp_camera_sensor_get();
    if (!s) return;
    s->set_vflip(s, cfg.camVflip ? 1 : 0);
    s->set_hmirror(s, cfg.camHmirror ? 1 : 0);
    s->set_quality(s, cfg.camQuality);
    if (psramFound()) s->set_framesize(s, sizeOf(cfg.camSize));
    // aquario costuma ter luz forte de cima: um leve ganho de brilho ajuda
    s->set_brightness(s, 1);
    s->set_saturation(s, 0);
}

bool cameraReady()          { return ready; }
const char *cameraLastError(){ return lastErr; }

#else  // ------------------------------------------------ sem camera

bool cameraInit()            { return false; }
bool cameraReady()           { return false; }
void cameraApplySettings()   {}
const char *cameraLastError(){ return "compilado sem camera"; }

#endif
