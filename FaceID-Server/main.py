from fastapi import FastAPI, HTTPException
from fastapi.responses import JSONResponse
from fastapi import File, UploadFile, Form
from pydantic import BaseModel
from PIL import Image
from dotenv import load_dotenv
import requests
import numpy as np
from io import BytesIO
import os
import json
from ultralytics import YOLO

from utils import detect_largest_face, embedding, cosine_similarity

app = FastAPI()

# Load environment variables from .env file
load_dotenv()




# SQLite setup for embeddings
import sqlite3
SQLITE_DB_PATH = 'face_embeddings.db'
conn = sqlite3.connect(SQLITE_DB_PATH)
c = conn.cursor()
c.execute('''CREATE TABLE IF NOT EXISTS face_embeddings (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT, student_id TEXT, embedding BLOB)''')
conn.commit()
conn.close()




@app.post("/add-face/")
async def add_face(name: str = Form(...), student_id: str = Form(...), image: UploadFile = File(...)):
    """
    Endpoint to process an uploaded image and save face embedding with the user's name and student_id.
    """
    try:
        img_bytes = await image.read()
        img = Image.open(BytesIO(img_bytes)).convert('RGB')
        img_np = np.array(img)

        # Detect the largest face
        try:
            print("Detecting the largest face...")
            largest_face = detect_largest_face(img_np, 'yolov8')
            if largest_face is None:
                print("No face detected")
                return JSONResponse(status_code=200, content={"result": "No face detected"})
        except Exception as e:
            print("Error during face detection:", str(e))
            return JSONResponse(status_code=200, content={"result": "No face detected", "error": str(e)})

        # Get the embedding of the detected face
        print("Generating embedding for the detected face...")
        face_embedding = embedding(largest_face, 'Facenet')
        print(f"Embedding generated: {face_embedding}")

        # Save the embedding to SQLite
        conn = sqlite3.connect(SQLITE_DB_PATH)
        c = conn.cursor()
        embedding_np = np.array(face_embedding, dtype=np.float32)
        c.execute('INSERT INTO face_embeddings (name, student_id, embedding) VALUES (?, ?, ?)', (name, student_id, embedding_np.tobytes()))
        face_id = c.lastrowid
        conn.commit()
        conn.close()
        print("Face embedding saved to SQLite DB successfully")

        return JSONResponse(content={"result": "Face detected and saved", "id": face_id, "name": name, "student_id": student_id, "embedding_shape": embedding_np.shape})

    except Exception as e:
        raise HTTPException(status_code=500, detail="An error occurred while processing the image: " + str(e))




@app.post("/verify-face/")
async def verify_face(image: UploadFile = File(...)):
    """
    Endpoint to verify a face from an uploaded image file.
    It compares the face in the image with known embeddings stored in the SQLite database.
    """
    try:
        img_bytes = await image.read()
        img = Image.open(BytesIO(img_bytes)).convert('RGB')
        img_np = np.array(img)

        # Detect the largest face in the image
        try:
            largest_face = detect_largest_face(img_np, 'yolov8')
            if largest_face is None:
                return JSONResponse(status_code=200, content={"result": "No face detected"})
        except Exception as e:
            print("Error during face detection:", str(e))
            return JSONResponse(status_code=200, content={"result": "No face detected", "error": str(e)})

        # Get the embedding of the detected face
        face_embedding = embedding(largest_face, 'Facenet')

        # Load known face embeddings from the SQLite database
        conn = sqlite3.connect(SQLITE_DB_PATH)
        c = conn.cursor()
        c.execute('SELECT id, name, student_id, embedding FROM face_embeddings')
        rows = c.fetchall()
        conn.close()

        closest_id = None
        closest_name = None
        closest_student_id = None
        closest_distance = -1

        for row in rows:
            db_id, db_name, db_student_id, db_embedding_blob = row
            db_embedding = np.frombuffer(db_embedding_blob, dtype=np.float32)
            distance = cosine_similarity(face_embedding, db_embedding)
            if distance > closest_distance and distance > 0.55:
                closest_distance = distance
                closest_id = db_id
                closest_name = db_name
                closest_student_id = db_student_id

        if closest_id is not None:
            return JSONResponse(content={
                "result": "Face matched",
                "closest_id": closest_id,
                "name": closest_name,
                "student_id": closest_student_id,
                "distance": closest_distance
            })
        else:
            return JSONResponse(content={"result": "No matching face found"})

    except Exception as e:
        raise HTTPException(status_code=500, detail="An error occurred while processing the image: " + str(e))

# Load YOLOv8 model
model = YOLO('yolov8n.pt')  # Bạn có thể thay đổi thành mô hình YOLOv8 lớn hơn nếu muốn

class ImageURL(BaseModel):
    image_url: str


@app.post("/detect-human/")
async def detect_human(image: UploadFile = File(...)):
    """
    Endpoint to check if there is a human in the uploaded image file.
    """
    try:
        img_bytes = await image.read()
        img = Image.open(BytesIO(img_bytes)).convert('RGB')
        img_np = np.array(img)

        # Use YOLO to detect humans in the image (class 0)
        results = model.predict(source=img_np, classes=0)

        # Check results
        if len(results[0].boxes) > 0:
            return JSONResponse(content={"result": "Human detected", "num_humans": len(results[0].boxes)})
        else:
            return JSONResponse(content={"result": "No human detected"})

    except Exception as e:
        raise HTTPException(status_code=500, detail="An error occurred while processing the image: " + str(e))