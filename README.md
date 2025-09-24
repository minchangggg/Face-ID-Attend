# Face ID Attend
<img width="150" alt="image" src="https://github.com/user-attachments/assets/0f3917de-76d2-441e-b777-fd996f000896">

## Introduction
- Face ID Attend is a IoT-based contact-free attendance system with face recognition — built two ways:
  + FaceID-ESP — ESP32-CAM + ESP-WHO (ultra-low cost, runs fully on-device)
  + FaceID-Pi — Raspberry Pi 5 + Pi Camera (more accuracy & room to grow)
- What it does
  + Detects & recognizes faces in real time
  + Logs check-in / check-out with timestamp
  + Stores locally or sends to a simple HTTP endpoint (CSV/JSON export)

### FaceID-ESP (ESP32-CAM + ESP-WHO)
- Flash ESP32-CAM with ESP-WHO firmware.
- Set Wi-Fi & (optional) server URL.
- Enroll faces → start recognition.

### FaceID-Pi (Raspberry Pi 5 + Pi Cam)
- Install Python deps (e.g., OpenCV / chosen face model).
- Connect Pi Camera, run the script.
- Enroll faces → start recognition.

## Ref
> https://www.youtube.com/watch?v=fHSl8G4zTkM
