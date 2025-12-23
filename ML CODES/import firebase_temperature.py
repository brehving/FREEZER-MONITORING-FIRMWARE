import firebase_admin
from firebase_admin import credentials, firestore
import pandas as pd

# Initialize Firebase
cred = credentials.Certificate("serviceAccountKey.json")
firebase_admin.initialize_app(cred)

db = firestore.client()

# 🔴 CHANGE THIS to your collection name
COLLECTION_NAME = "TEMPERATURE"

docs = db.collection(COLLECTION_NAME).stream()

rows = []
for doc in docs:
    data = doc.to_dict()
    data["document_id"] = doc.id
    rows.append(data)

df = pd.DataFrame(rows)

df.to_csv(f"{COLLECTION_NAME}.csv", index=False)

print("✅ CSV exported successfully!")
