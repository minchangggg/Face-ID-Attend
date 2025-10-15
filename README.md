# Face ID Attend
<img width="1919" height="487" alt="image" src="https://github.com/user-attachments/assets/81c4da88-6e1d-4153-80a3-a12b5527b093" />

## 1. What it does
- **Face ID Attend** is an IoT-based, contact-free attendance system with face recognition.
  - Real-time face detection & recognition
  - Touchless **Check-In / Check-Out** with timestamps
  - Logging to **CSV/JSON** locally or **HTTP POST** to a server

-------------------------

## 2. Approaches

### 2.1. `Approach 1: On-device (ESP32-CAM + ESP-WHO)`
> Folder Code: **FaceID-ESP-CAM**
- Detection & recognition run entirely on the ESP32-CAM.
- Lowest cost; no network required after enrollment.
- Best for small classrooms/kiosks with 1–2 faces in frame.

### 2.2. `Approach 2: Edge-Server (ESP32-CAM → Server/Laptop)` => main approach
> Folder Code: **FaceID-Server**
- ESP32-CAM streams MJPEG/RTSP/HTTP frames to a local server or laptop.
- Server performs detection/recognition (e.g., YOLO/FaceNet).
- Scales to more users/cameras; higher accuracy and speed than on-device.

### 2.3. `Approach 3: Standalone Edge (Raspberry Pi 5 + Pi Camera v2)`
> Folder Code: **FaceID-PI**
- All processing on the Pi (detect, embed, match, log).
- No external server required; higher performance than ESP32-only.
- Good balance of accuracy, cost, and maintainability.

### 2.4. Comparison of Approaches

| Dimension                | On-device (ESP32-CAM + ESP-WHO)              | Edge-Server (ESP32-CAM → Server/Laptop)                 | Standalone Edge (Raspberry Pi 5 + Pi Camera v2)         |
|---|---|---|---|
| **Hardware cost**        | **Lowest**                                   | Low–Medium (ESP32 + existing PC/laptop/server)          | Medium (Pi 5 + camera)                                  |
| **Accuracy / Models**    | Basic (lightweight ESP-WHO)                  | **High** (RetinaFace/YOLO + ArcFace/FaceNet on CPU/GPU) | High (OpenCV/ONNX/NCNN on Pi)                           |
| **Throughput (FPS\*)**   | ~3–8 FPS (QVGA/VGA, 1–2 faces)               | **10–30+ FPS** (depends on model/GPU)                   | ~8–20 FPS (depends on optimizations)                    |
| **Latency**              | Low–Medium (on-device)                       | Medium (network + server processing)                    | Low–Medium (on-device)                                  |
| **Scalability**          | Limited (1 camera per node)                  | **High** (multi-camera, central compute)                | Medium (per-node scale; add more Pis)                   |
| **Network dependency**   | None after enrollment                        | Required (streaming to server)                          | Optional (local logging; optional POST)                 |
| **Offline operation**    | **Yes**                                      | No (server required)                                    | **Yes**                                                 |
| **Setup complexity**     | Low (flash firmware, basic config)           | Medium (camera stream + server stack/models)            | Medium (OS + camera + libs)                             |
| **Maintenance**          | Firmware updates per device                  | Centralized model/app updates                           | Per-node app updates                                    |
| **Power**                | **Very low**                                 | Low (ESP) + higher on server                            | Moderate (Pi 5)                                         |
| **Best for**             | Ultra-low-cost kiosks, small rooms           | Campuses with many cameras, higher accuracy needs       | Classrooms/offices without server, balanced performance |
| **Key limitations**      | Lower accuracy, limited faces in frame       | Needs stable LAN; server sizing & security              | Cost/power > ESP; still below desktop-class performance |

\* *FPS values are indicative and depend on resolution, model, and optimizations.*

-------------------------

## 3. Execution Flow

### 3.1. Overview
FaceID operates in two main phases: **Enrollment** and **Recognition & Attendance**.

**Phase 1 — Enrollment**
1. **Capture:** Client captures face images (ESP32-CAM or Pi Camera).
2. **Process:** `detect → align → embed` → produce embedding vector **`d`**.
3. **Store:** Save `{person_id, name, d, meta}` to the face database (DB).

**Phase 2 — Recognition & Attendance**
1. **Frame Capture:** Take a frame from the camera stream.
2. **Per Face:** `detect → align → embed` to get query vector **`q`**.
3. **Match:** Compare **`q`** to stored embeddings (e.g., cosine similarity).  
   - If `score ≥ threshold` → fetch `{name, student_id}`.  
   - Else → mark as **unknown** (optional: save for later review).
4. **Log Event:** Write the attendance record and update **`PeopleCount`**.

### 3.2. System Architecture & Data Flow (Approach 2)
*(Edge-Server: ESP32-CAM → Server/Laptop)*

- **ESP32-CAM** streams frames (MJPEG/RTSP/HTTP) → **FastAPI Server**.
- **Detecting Service**: face detection + alignment → crops/landmarks.
- **Embedding Service**: generates embeddings → **`d`** / **`q`** vectors.
- **SQLite DB**: stores embeddings and metadata; queried on match.
- **Data Ingesting Service**: logs events to CSV/JSON and exposes HTTP endpoints.
- **UI (optional)**: enrollment/review dashboard.

<img width="3953" height="1586" alt="faceID" src="https://github.com/user-attachments/assets/b4ecfdf5-b6a9-43c7-8e7d-e273c98dc0ca" />


