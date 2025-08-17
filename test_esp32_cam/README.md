# ESP32 CAM
> Docs https://www.youtube.com/watch?v=RCtVxZnjPmY

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

### 1. Các Thư Viện và Định Nghĩa
- #include "esp_camera.h": Bao gồm thư viện ESP32 để điều khiển camera OV2640. Thư viện này cung cấp các hàm khởi tạo và cấu hình camera.
- #include <WiFi.h>: Bao gồm thư viện WiFi để kết nối ESP32-CAM với mạng WiFi, cần thiết cho streaming qua server.
- #define CAMERA_MODEL_AI_THINKER: Xác định mô hình camera là AI-Thinker ESP32-CAM, một biến thể phổ biến có PSRAM. Định nghĩa này giúp chọn các chân GPIO phù hợp (được định nghĩa trong camera_pins.h).
- #include "camera_pins.h": Bao gồm tệp cấu hình các chân GPIO cho mô hình AI-Thinker (ví dụ: Y2_GPIO_NUM, XCLK_GPIO_NUM, v.v.), phù hợp với cách kết nối của camera OV2640.
- const char *ssid = "Minh Trang"; và const char *password = "31122003";: Định nghĩa tên mạng WiFi (SSID) và mật khẩu để ESP32-CAM kết nối. Đây là thông tin cá nhân của bạn, cần đảm bảo chính xác để kết nối thành công.
### 2. Khai Báo Hàm
- void startCameraServer();: Khai báo hàm để khởi động server streaming hình ảnh qua WiFi. Hàm này thường được định nghĩa trong thư viện ESP32 hoặc cần bổ sung mã riêng để xử lý (bạn có thể đã sử dụng thư viện như app_httpd).
- void setupLedFlash(int pin);: Khai báo hàm để cấu hình đèn LED flash (nếu có) trên ESP32-CAM, thường dùng để chiếu sáng khi chụp ảnh trong điều kiện thiếu sáng.
### 3. Hàm setup()
#### Hàm setup() chạy một lần khi ESP32-CAM khởi động, cấu hình phần cứng và kết nối mạng.
- Serial.begin(115200); Serial.setDebugOutput(true); Serial.println();:
  
        Khởi động giao tiếp Serial với tốc độ 115200 baud để in thông tin debug.
        setDebugOutput(true) bật chế độ in thông tin debug chi tiết.
        Serial.println() in một dòng trống để làm rõ đầu ra.
  
- Serial.print("PSRAM found: "); Serial.println(psramFound() ? "Yes" : "No");:

        Kiểm tra và in trạng thái PSRAM (Pseudo-Static RAM) trên ESP32-CAM.
        psramFound() trả về true nếu PSRAM được phát hiện (AI-Thinker ESP32-CAM có PSRAM), giúp lưu trữ frame buffer lớn hơn.

- camera_config_t config;: Khai báo cấu trúc camera_config_t để định nghĩa các thông số của camera.

- **Cấu hình Pin và Thông Số Camera:**
  
        config.ledc_channel = LEDC_CHANNEL_0; config.ledc_timer = LEDC_TIMER_0;: Sử dụng kênh LEDC (LED Control) 0 và timer 0 để điều khiển xung cho camera.
        config.pin_d0 đến config.pin_reset: Gán các chân GPIO cho dữ liệu (D0-D7), xung đồng bộ (PCLK, VSYNC, HREF), và giao tiếp I2C (SCCB_SDA, SCCB_SCL). Các giá trị như Y2_GPIO_NUM được định nghĩa trong camera_pins.h cho mô hình AI-Thinker.
        config.xclk_freq_hz = 20000000;: Tần số xung XCLK là 20 MHz, phù hợp với OV2640.
        config.frame_size = FRAMESIZE_QVGA;: Đặt độ phân giải ban đầu là QVGA (320x240), một kích thước nhỏ để giảm tải bộ nhớ.
        config.pixel_format = PIXFORMAT_JPEG;: Định dạng đầu ra là JPEG, phù hợp cho streaming qua mạng.
        config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;: Chế độ chụp ảnh khi buffer trống, tối ưu cho streaming liên tục.
        config.fb_location = CAMERA_FB_IN_PSRAM;: Lưu frame buffer trong PSRAM nếu có, giúp tăng hiệu suất.
        config.jpeg_quality = 12; config.fb_count = 1;: Chất lượng JPEG là 12 (chất lượng trung bình, số càng thấp càng cao), và chỉ sử dụng 1 frame buffer.

- if (psramFound()) { ... } else { ... }:
  
        + Nếu PSRAM được phát hiện:
          Giảm jpeg_quality xuống 10 (chất lượng cao hơn).
          Tăng fb_count lên 2 và đổi grab_mode thành CAMERA_GRAB_LATEST (lấy frame mới nhất, giảm độ trễ).
        + Nếu không có PSRAM: Giữ frame_size là SVGA (800x600) và lưu buffer trong DRAM.

- esp_err_t err = esp_camera_init(&config);: Khởi tạo camera với cấu hình đã định nghĩa. Nếu thất bại (err != ESP_OK), in lỗi ra Serial và thoát hàm (có thể do camera không được kết nối đúng hoặc PSRAM không đủ).
- sensor_t *s = esp_camera_sensor_get();: Lấy con trỏ đến cấu hình cảm biến OV2640 để điều chỉnh các thông số.
- if (s->id.PID == OV3660_PID) { ... }:

        Kiểm tra ID cảm biến. Tuy nhiên, OV2640 có PID là 0x2642, không phải 0x3660, nên đoạn code này không chạy. Đây là lỗi logic, cần sửa hoặc xóa.
        Nếu chạy, nó sẽ lật dọc (vflip), tăng độ sáng (brightness), và giảm bão hòa màu (saturation).

- s->set_framesize(s, FRAMESIZE_QVGA);: Ghi đè độ phân giải thành QVGA (320x240), đảm bảo nhất quán với cấu hình ban đầu.
- #if defined(CAMERA_MODEL_AI_THINKER) ... #endif: Nếu định nghĩa CAMERA_MODEL_AI_THINKER, lật dọc (vflip) và lật ngang (hmirror) hình ảnh. Điều này phù hợp với cách camera AI-Thinker được lắp đặt.
- #if defined(LED_GPIO_NUM) ... #endif: Nếu chân LED được định nghĩa (thường là GPIO 4 trên AI-Thinker), gọi hàm setupLedFlash để cấu hình đèn flash.

- **Kết Nối WiFi:**

        WiFi.begin(ssid, password); WiFi.setSleep(false);: Khởi động kết nối WiFi với SSID và mật khẩu đã cho, tắt chế độ ngủ để đảm bảo kết nối ổn định.
        while (WiFi.status() != WL_CONNECTED) { ... }: Vòng lặp đợi đến khi kết nối WiFi thành công. In dấu chấm (.) mỗi 500ms để theo dõi tiến trình. Nếu không kết nối được, vòng lặp chạy vô hạn (có thể gây treo, nên thêm timeout là tốt hơn).
        Serial.println("WiFi connected");: In thông báo khi kết nối thành công.

- startCameraServer();: Khởi động server để streaming hình ảnh qua địa chỉ IP. Hàm này thường được định nghĩa trong thư viện ESP32 (ví dụ: app_httpd.cpp), cung cấp giao diện web để xem hình ảnh.
- Serial.print("Camera Ready! Use 'http://"); Serial.print(WiFi.localIP()); Serial.println("' to connect");: In thông báo và địa chỉ IP để truy cập camera qua trình duyệt (ví dụ: http://192.168.1.x).
### 4. Hàm loop()
delay(10000);: Chạy vô hạn với độ trễ 10 giây giữa các lần lặp. Hàm này không làm gì nhiều vì mọi chức năng chính (camera, WiFi, server) đã được xử lý trong setup().

## Điểm Cần Lưu Ý và Cải Thiện
- Lỗi Logic trong if (s->id.PID == OV3660_PID): OV2640 có PID khác (0x2642), nên đoạn code này không chạy. Nên xóa hoặc sửa thành if (s->id.PID == OV2640_PID) (hoặc bỏ kiểm tra nếu chỉ dùng OV2640).
- Vòng Lặp WiFi Không Có Timeout: Nếu WiFi không kết nối được, code sẽ treo. Nên thêm giới hạn (ví dụ: 10 lần thử, như trong code đề xuất trước).
- Chất Lượng Camera: Độ phân giải QVGA (320x240) và jpeg_quality = 12 (hoặc 10) là trung bình. Có thể tăng lên SVGA (800x600) và giảm jpeg_quality xuống 5 để cải thiện (như trong code đề xuất trước).
- Hàm startCameraServer() và setupLedFlash: Những hàm này cần được định nghĩa hoặc bao gồm từ thư viện (thường từ ví dụ ESP32-CAM của Arduino). Nếu chưa có, bạn cần thêm mã nguồn từ tài liệu ESP32.
- Tóm Tắt Cách Hoạt Động:
  
        Code khởi động ESP32-CAM, kiểm tra PSRAM, cấu hình camera OV2640 với độ phân giải 320x240 và định dạng JPEG.
        Điều chỉnh một số thông số cảm biến (lật hình, độ sáng, v.v.) dựa trên mô hình AI-Thinker.
        Kết nối WiFi với mạng "Minh Trang" và mật khẩu "31122003".
        Khởi động server streaming và cung cấp địa chỉ IP để xem hình ảnh qua trình duyệt.
        Chạy vô hạn với độ trễ 10 giây, nhưng không có chức năng bổ sung trong loop().
