import os
import numpy as np
from PIL import Image
from ultralytics import YOLO
import tensorflow as tf

# Paths
MODEL_DIR = os.path.dirname(__file__)
YOLO_PATH = os.path.join(MODEL_DIR, '../faceid-model/yolov8n-face.pt')
FACENET_PATH = os.path.join(MODEL_DIR, '../faceid-model/facenet_weights.h5')

# Load models
print("Loading YOLOv8 face detector...")
detector = YOLO(YOLO_PATH)
print("Loading FaceNet...")
facenet = tf.keras.models.load_model(FACENET_PATH, compile=False)

# Helper: align face using landmarks (placeholder, needs real implementation)
def align_face(img, landmarks, output_size=(160, 160)):
    # img: PIL Image, landmarks: np.array shape (5,2)
    # TODO: implement similarity transform using landmarks
    # For now, just crop a square around the first box as a placeholder
    return img.resize(output_size)

# Example usage
img_path = "test.jpg"  # Replace with your image path
img = Image.open(img_path).convert('RGB')
img_np = np.array(img)

# Detect faces
results = detector(img_np)
for result in results:
    boxes = result.boxes.xyxy.cpu().numpy()  # (N, 4)
    landmarks = result.keypoints.cpu().numpy()  # (N, 5, 2)
    for i, (box, lm) in enumerate(zip(boxes, landmarks)):
        # Align face
        aligned = align_face(img, lm, output_size=(160, 160))
        aligned_np = np.array(aligned).astype(np.float32)
        aligned_np = aligned_np / 255.0
        aligned_np = np.expand_dims(aligned_np, axis=0)
        # Embed
        emb = facenet(aligned_np)
        print(f"Face {i}: embedding shape {emb.shape}")
