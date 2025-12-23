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

TEMP_df = pd.DataFrame(rows)


COLLECTION_NAME = "EVAPTEMP"

evaptemp_docs = db.collection(COLLECTION_NAME).stream()

rows = []
for doc in evaptemp_docs:
    data = doc.to_dict()
    data["document_id"] = doc.id
    rows.append(data)

EVAPTEM_DF = pd.DataFrame(rows)

COLLECTION_NAME = "DOOR"

DOOR_docs = db.collection(COLLECTION_NAME).stream()

rows = []
for doc in DOOR_docs:
    data = doc.to_dict()
    data["document_id"] = doc.id
    rows.append(data)

DOOR_DF = pd.DataFrame(rows)



DF_combined = pd.concat([TEMP_df,EVAPTEM_DF,DOOR_DF], ignore_index=True)

DF_combined.to_csv("ABC2.csv", index=False)


print("✅ CSV exported successfully!")
