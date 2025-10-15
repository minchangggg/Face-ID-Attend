# FaceID-ESP [FaceID — On-Device (ESP32-CAM + ESP-WHO)]
## Overview
- This module runs **face detection & recognition entirely on the ESP32-CAM** using ESP-WHO.  
- No external server is required after enrollment. Ideal for ultra-low-cost kiosks and small rooms.

> https://www.14core.com/esp32-s-cam-in-face-detection-and-recognition-with-esp-who-library/?utm_source=chatgpt.com

## Requirements
- Board: ESP32-CAM (AI-Thinker or equivalent).

<img width="270" alt="image" src="https://github.com/user-attachments/assets/11721ba0-5ed0-4caf-aa5c-fe7d4cb2508d">
<img width="140" alt="image" src="https://github.com/user-attachments/assets/13878d8e-575d-44d9-9d27-af9a1379ff66">
<img width="150" alt="image" src="https://github.com/user-attachments/assets/9fb330d6-ecaf-4c63-876b-d1e720a2472b">
<img width="147" alt="image" src="https://github.com/user-attachments/assets/cb5ab32f-0c67-4d5a-84ff-2e23b28af88f">

- Toolchain: **ESP-IDF 4.4.7** with `idf.py`.
> https://dl.espressif.com/dl/esp-idf/?idf=4.4

- **ESP-WHO v1.0.0**.
> https://github.com/espressif/esp-who.git -b v1.0.0

- Stable 5V supply (≥ 1A recommended). Avoid brownouts during Wi-Fi + camera.

---

## Folder Layout
```
FaceID-ESP-CAM/
├─ FaceID_ESP_WHO/ # Main on-device FaceID app (ESP-WHO based)
├─ Test_ESP32_CAM/ # Minimal camera + web server test (stream/snapshot)
└─ Test_IDF_ESP_5.5/ # ESP-IDF 5.5 toolchain + board sanity tests
```

- **FaceID_ESP_WHO**: production codepath; performs `detect → align → embed → match` on-device; logs attendance.
- **Test_ESP32_CAM**: quick check that the module, lens, and Wi-Fi work (e.g., open a browser and see the stream/snapshot).
- **Test_IDF_ESP_5.5**: verifies toolchain/USB/flash/monitor; use before building the main app if setup is fresh.

## How to run
### Camera + Wi-Fi test (web server)
```
cd ../Test_ESP32_CAM
idf.py set-target esp32
idf.py menuconfig   # set Wi-Fi SSID/PASS if provided in Kconfig
idf.py build flash monitor
```

### Build the main FaceID app
```
cd ../FaceID_ESP_WHO
idf.py set-target esp32
idf.py menuconfig   # set Wi-Fi, logging options, thresholds
idf.py build flash monitor
```

