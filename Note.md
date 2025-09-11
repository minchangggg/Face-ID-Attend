## Cách 1. Đơn giản nhất (ESP32-CAM tự lo hết)
- ESP32-CAM stream hình → dùng thư viện ESP-WHO (face detection/recognition) của Espressif.
- Nó nhận diện khuôn mặt trực tiếp trên ESP32 (so sánh với khuôn mặt đã đăng ký).
- Khi nhận diện thành công → ESP32-CAM gửi tên/số MSSV qua WiFi → Server (hoặc Google Sheet).
- Ưu điểm: không cần thiết bị khác.
- Nhược: ESP32-CAM khá yếu, tốc độ nhận diện chậm, chỉ ổn với lớp nhỏ (10–20 người).
------
## Cách 2. Cách phổ biến (ESP32-CAM chỉ để chụp, xử lý ở thiết bị mạnh hơn)
- ESP32-CAM chỉ chụp ảnh hoặc stream video.
- Gửi ảnh về Raspberry Pi 5 / Laptop để chạy model AI (OpenCV + face_recognition).
- Server xử lý nhanh → trả kết quả (tên người).
- Server đồng thời lưu log điểm danh vào file Excel, Google Sheet, hoặc Database.
- Ưu điểm: chính xác, nhanh, dễ mở rộng cho lớp 50–100 người.
- Nhược: cần thêm 1 máy trung gian (Pi5 hoặc laptop).
------
## Mở rộng 
- Xuất hình ảnh / file excel qua màn hình di động 

