# Cách 1: Đơn giản nhất (ESP32-CAM tự lo hết)
> Với **Cách 1** (ESP32-CAM tự lo hết), toàn bộ việc **nhận diện khuôn mặt** chạy trực tiếp **trên ESP32-CAM**, dùng **ESP-WHO framework** (Espressif cung cấp).
## Nguyên lý hoạt động
* ESP32-CAM vừa **stream hình** vừa chạy **thư viện ESP-WHO** (do Espressif phát triển).
* Thư viện này có sẵn chức năng **face detection** (phát hiện) và **face recognition** (nhận diện).
* Khuôn mặt được so sánh trực tiếp ngay trên ESP32 với các khuôn mặt đã đăng ký.
* Khi nhận diện thành công → ESP32-CAM gửi **thông tin tên hoặc MSSV** qua WiFi lên **server** (hoặc ghi trực tiếp vào Google Sheet).
## Ưu điểm
* Chỉ cần một thiết bị ESP32-CAM, **không cần thêm Raspberry Pi hay laptop**.
* Setup đơn giản, triển khai nhanh.
## Nhược điểm
* ESP32-CAM có **tài nguyên hạn chế** (RAM \~512KB, CPU \~240MHz).
* Tốc độ nhận diện chậm, đặc biệt khi số lượng khuôn mặt lớn.
* Chỉ phù hợp lớp nhỏ (tầm 10–20 người).
## Cách chạy thực tế
* Cậu sẽ phải build code bằng **ESP-IDF** (terminal IDF environment).
  * Clone repo **esp-who** về:
    ```bash
    git clone https://github.com/espressif/esp-who.git
    ```
  * Vào folder ví dụ, ví dụ như `examples/human_face_recognition/`
  * Dùng lệnh build và flash:
    ```bash
    idf.py set-target esp32
    idf.py menuconfig   # chỉnh wifi SSID, password
    idf.py build
    idf.py -p /dev/ttyUSB0 flash monitor
    ```
* Sau khi flash, ESP32-CAM sẽ chạy firmware → mở camera → detect/recognize ngay trên chip.
## Tóm lại:
* **Nơi chạy:** Trên **ESP32-CAM** (không cần PC hay server xử lý).
* **Công cụ build:** **ESP-IDF (terminal)**, không dùng Arduino IDE trong cách này (vì esp-who viết cho IDF).
* **Output:** Khi nhận diện thành công → ESP32-CAM log ra serial monitor, đồng thời có thể gửi tên/MSSV qua **WiFi** lên server hoặc Google Sheet.


---

# Cách 2: Phổ biến và mạnh hơn (ESP32-CAM chỉ chụp, xử lý ở thiết bị mạnh hơn)
> Với **Cách 2** ESP32-CAM chỉ làm "camera", xử lý ở thiết bị mạnh hơn
## Nguyên lý hoạt động
* ESP32-CAM chỉ đóng vai trò **camera IP**: chụp ảnh hoặc stream video.
* Hình ảnh được gửi về một **thiết bị trung gian mạnh hơn** như Raspberry Pi 5 hoặc laptop.
* Thiết bị trung gian chạy **AI model** (ví dụ: OpenCV + thư viện `face_recognition`).
* Kết quả nhận diện (tên người) được xử lý và gửi về **server/Google Sheet/Database** để lưu log điểm danh.
## Ưu điểm
* Xử lý nhanh hơn nhiều so với ESP32-CAM.
* Độ chính xác cao hơn (sử dụng mô hình AI nặng hơn).
* Dễ dàng mở rộng cho lớp đông (50–100 người).
## Nhược điểm
* Cần thêm một thiết bị trung gian (Pi5 hoặc laptop).
* Phức tạp hơn trong việc setup (phải kết nối ESP32-CAM ↔ server).
## Nơi chạy
### ESP32-CAM: chỉ chạy firmware nhẹ (chụp ảnh, stream video qua WiFi).
- Code này có thể viết bằng Arduino IDE hoặc ESP-IDF, nhưng không cần thư viện nặng như ESP-WHO.
- Nhiệm vụ chính: gửi ảnh/frame lên server (Raspberry Pi 5 hoặc laptop).
### Server (Raspberry Pi / Laptop): nơi chạy model AI (ví dụ: OpenCV + thư viện face_recognition của Python).
- Model này sẽ load database khuôn mặt đã đăng ký, rồi so sánh ảnh nhận được từ ESP32-CAM.
- Khi nhận diện thành công → server lưu log điểm danh (Excel, Google Sheet, hoặc Database).
## Cách chạy thực tế
### ESP32-CAM (Arduino/IDF terminal)
- Upload code để ESP32-CAM stream video hoặc chụp ảnh gửi HTTP.
- Ví dụ với Arduino, dùng thư viện ESP32-CAM CameraWebServer để mở stream http://<esp32-ip>:81/stream.
### Server (Python, chạy trên terminal máy chủ)
- Viết code Python với OpenCV + face_recognition:
  ```py
  import cv2, face_recognition
  ```
- Server lấy ảnh từ ESP32-CAM → detect & recognize.
- Nếu nhận diện thành công, server ghi log (Excel, Google Sheet API, hoặc DB).

---

- Tóm lại:
  + Nếu muốn **đơn giản, gọn nhẹ, chỉ 1 con ESP32-CAM** → chọn **Cách 1**.
  + Nếu cần **tốc độ, chính xác, mở rộng cho lớp đông** → nên chọn **Cách 2** với Raspberry Pi 5 hoặc laptop làm server.

---
