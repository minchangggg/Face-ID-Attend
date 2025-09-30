import os
import numpy as np
import sqlite3
from fastapi import FastAPI, File, UploadFile, Form
import zipfile
from io import BytesIO
from keras.models import load_model
from keras.applications.imagenet_utils import preprocess_input
from PIL import Image

app = FastAPI()

# Load FaceNet model once
MODEL_PATH = os.path.join(os.path.dirname(__file__), '../faceid-model/facenet_weights.h5')
model = load_model(MODEL_PATH)

# SQLite setup
DB_PATH = os.path.join(os.path.dirname(__file__), 'embeddings.db')
conn = sqlite3.connect(DB_PATH)
c = conn.cursor()
# Add id as auto-increment primary key
c.execute('''CREATE TABLE IF NOT EXISTS faces (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT, embedding BLOB)''')
conn.commit()
conn.close()

# Single image registration endpoint (restored)
@app.post("/register-face/")
async def register_face(name: str = Form(...), image: UploadFile = File(...)):
    img_bytes = await image.read()
    from io import BytesIO
    img_array = preprocess_image_for_facenet(BytesIO(img_bytes))
    embedding = model.predict(img_array)[0]
    conn = sqlite3.connect(DB_PATH)
    c = conn.cursor()
    c.execute('INSERT INTO faces (name, embedding) VALUES (?, ?)', (name, embedding.tobytes()))
    face_id = c.lastrowid
    conn.commit()
    conn.close()
    return {"id": face_id, "name": name, "embedding_shape": embedding.shape}

@app.get("/list-faces/")
def list_faces():
    conn = sqlite3.connect(DB_PATH)
    c = conn.cursor()
    c.execute('SELECT id, name, embedding FROM faces')
    rows = c.fetchall()
    conn.close()
    faces = []
    for face_id, name, embedding_blob in rows:
        embedding = np.frombuffer(embedding_blob, dtype=np.float32)
        faces.append({
            "id": face_id,
            "name": name,
            "embedding_shape": embedding.shape,
        })
    return {"faces": faces}

def preprocess_image_for_facenet(image_bytes, target_size=(160, 160)):
    img = Image.open(image_bytes).convert('RGB')
    img = img.resize(target_size)
    img_array = np.asarray(img, dtype=np.float32)
    img_array = np.expand_dims(img_array, axis=0)
    img_array = preprocess_input(img_array)
    return img_array


# New endpoint: register multiple faces from a zip file
@app.post("/register-faces-zip/")
async def register_faces_zip(name: str = Form(...), zip_file: UploadFile = File(...)):
    zip_bytes = await zip_file.read()
    with zipfile.ZipFile(BytesIO(zip_bytes)) as archive:
        image_filenames = [f for f in archive.namelist() if f.lower().endswith((".png", ".jpg", ".jpeg"))]
        results = []
        conn = sqlite3.connect(DB_PATH)
        c = conn.cursor()
        for img_name in image_filenames:
            with archive.open(img_name) as img_file:
                img_array = preprocess_image_for_facenet(img_file)
                embedding = model.predict(img_array)[0]
                c.execute('INSERT INTO faces (name, embedding) VALUES (?, ?)', (name, embedding.tobytes()))
                face_id = c.lastrowid
                results.append({"id": face_id, "image": img_name, "embedding_shape": embedding.shape})
        conn.commit()
        conn.close()
    return {"name": name, "images_processed": len(results), "details": results}
