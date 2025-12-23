import firebase_admin
from firebase_admin import credentials, firestore
import pandas as pd

# Initialize Firebase
cred = credentials.Certificate("serviceAccountKey.json")
firebase_admin.initialize_app(cred)

db = firestore.client()



COLLECTION_NAME = "DOOR"

DOOR_docs = db.collection(COLLECTION_NAME).stream()

rows = []
for doc in DOOR_docs:
    data = doc.to_dict()
    data["document_id"] = doc.id
    rows.append(data)

DOOR_DF = pd.DataFrame(rows)




DOOR_DF.to_csv("ABC2.csv", index=False)


print("✅ CSV exported successfully!")
