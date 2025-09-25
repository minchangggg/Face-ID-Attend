# Face ID Attend
<img width="130" alt="image" src="https://github.com/user-attachments/assets/0f3917de-76d2-441e-b777-fd996f000896">

## Introduction
- Face ID Attend is an IoT-based, contact-free attendance system with face recognition — built in two variants:
  + FaceID-ESP — ESP32-CAM + ESP-WHO (ultra-low cost, fully on-device)
  + FaceID-PI — Raspberry Pi + Pi Camera (higher accuracy & more headroom)
- What it does
  + Detects & recognizes faces in real time
  + Logs check-in / check-out with timestamp
  + Stores locally or sends to a simple HTTP endpoint (CSV/JSON export)

### FaceID-ESP (ESP32-CAM + ESP-WHO)
- Flash ESP32-CAM with ESP-WHO firmware.
- Configure Wi-Fi and (optional) server URL.
- Enroll faces → start recognition.

### FaceID-Pi (Raspberry Pi + Pi Cam)
- Install Python deps (e.g., OpenCV / chosen face model).
- Connect Pi Camera, run the script.
- Enroll faces → start recognition.

## Ref
> https://www.youtube.com/watch?v=fHSl8G4zTkM
