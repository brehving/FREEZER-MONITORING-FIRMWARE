import cv2
import pytesseract
import os
import json
import paho.mqtt.client as mqtt

# ---------------- TESSERACT CONFIG ----------------
pytesseract.pytesseract.tesseract_cmd = r"C:\Users\Vinay Karthik G\tesseract.exe"
os.environ["TESSDATA_PREFIX"] = r"C:\Users\Vinay Karthik G\tessdata"

# ---------------- MQTT CONFIG ----------------
MQTT_BROKER = "broker.hivemq.com"
MQTT_PORT = 1883
MQTT_TOPIC = "freezer/flavours"

# 👇 THIS LINE FIXES YOUR ERROR
client = mqtt.Client(client_id="flavour_publisher",
                     protocol=mqtt.MQTTv311,
                     callback_api_version=1)

# ---------------- IMAGE FOLDER ----------------
IMAGE_FOLDER = "images"

# ---------------- FLAVOURS ----------------
FLAVOURS = [
    "vanilla",
    "chocolate",
    "strawberry",
    "mango",
    "butterscotch",
    "pista",
    "coffee"
]

# ---------------- OCR PROCESS ----------------
flavour_count = {f: 0 for f in FLAVOURS}

for file in os.listdir(IMAGE_FOLDER):
    if file.lower().endswith((".jpg", ".jpeg", ".png")):
        path = os.path.join(IMAGE_FOLDER, file)
        img = cv2.imread(path)

        if img is None:
            continue

        gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
        text = pytesseract.image_to_string(gray, config="--psm 6").lower()

        for f in FLAVOURS:
            flavour_count[f] += text.count(f)

# ---------------- REMOVE ZERO COUNTS ----------------
flavour_count = {k: v for k, v in flavour_count.items() if v > 0}

payload = json.dumps(flavour_count)
print("📤 Publishing:", payload)

# ---------------- MQTT PUBLISH ----------------
client.connect(MQTT_BROKER, MQTT_PORT, 60)
client.publish(MQTT_TOPIC, payload)
client.disconnect()

print("✅ MQTT publish successful")