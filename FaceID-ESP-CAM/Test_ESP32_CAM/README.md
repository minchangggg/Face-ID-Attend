# ESP32 CAM
> Ref https://www.youtube.com/watch?v=RCtVxZnjPmY

## [My Result]
![image](https://github.com/user-attachments/assets/d2c42b76-7b17-46d1-a5b6-51e0ba5de061)

### Code
```c
#include "esp_camera.h"
#include <WiFi.h>

// Select camera model
#define CAMERA_MODEL_AI_THINKER // Has PSRAM
#include "camera_pins.h"

// Enter your WiFi credentials
const char *ssid = "Minh Trang";
const char *password = "31122003";

void startCameraServer();
void setupLedFlash(int pin);

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  // Debug PSRAM
  Serial.print("PSRAM found: ");
  Serial.println(psramFound() ? "Yes" : "No");

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.frame_size = FRAMESIZE_SVGA; // Tăng lên SVGA (800x600)
  config.pixel_format = PIXFORMAT_JPEG;
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 5; // Tăng chất lượng JPEG
  config.fb_count = 2;     // Tăng frame buffer

  if (psramFound()) {
    config.jpeg_quality = 5; // Giữ chất lượng cao
    config.fb_count = 3;     // Tăng frame buffer nếu có PSRAM
    config.grab_mode = CAMERA_GRAB_LATEST;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.fb_location = CAMERA_FB_IN_DRAM;
  }

  // Camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x\n", err);
    return;
  }

  sensor_t *s = esp_camera_sensor_get();
  s->set_brightness(s, 0);  // Độ sáng trung bình
  s->set_contrast(s, 1);    // Tăng độ tương phản
  s->set_saturation(s, 0);  // Màu sắc tự nhiên
  s->set_sharpness(s, 2);   // Tăng độ sắc nét
  s->set_denoise(s, 1);     // Giảm nhiễu
  s->set_framesize(s, FRAMESIZE_SVGA); // Đảm bảo độ phân giải
  s->set_vflip(s, 1);       // Lật dọc
  s->set_hmirror(s, 1);     // Lật ngang

#if defined(CAMERA_MODEL_AI_THINKER)
  s->set_vflip(s, 1);
  s->set_hmirror(s, 1);
#endif

#if defined(LED_GPIO_NUM)
  setupLedFlash(LED_GPIO_NUM);
#endif

  WiFi.begin(ssid, password);
  WiFi.setSleep(false);

  Serial.print("WiFi connecting");
  int retries = 10;
  while (retries > 0 && WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    retries--;
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.println("WiFi connected");
  } else {
    Serial.println("");
    Serial.println("WiFi connection failed");
    return;
  }

  startCameraServer();

  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");
}

void loop() {
  delay(10000);
}
```
