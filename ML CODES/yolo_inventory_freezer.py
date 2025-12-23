import cv2
import time
import paho.mqtt.client as mqtt
from ultralytics import YOLO

# ---------------- MQTT CONFIG ----------------
MQTT_BROKER = "broker.hivemq.com"
MQTT_TOPIC  = "freezer/inventory"

client = mqtt.Client()
client.connect(MQTT_BROKER, 1883, 60)

# ---------------- YOLO MODEL ----------------
model = YOLO("yolov8n.pt")   # lightweight & fast

# ---------------- CAMERA ----------------
cap = cv2.VideoCapture(0)

if not cap.isOpened():
    print("Camera not detected")
    exit()

print("YOLO Ice-Cream Inventory System Started")

# ---------------- ICE-CREAM CLASS MAPPING ----------------
# These YOLO labels will be treated as ICE CREAM
ICE_CREAM_ALIASES = [
    "bowl",
    "cup",
    "bottle",
    "book",
    "remote"
]

# ---------------- TIMING CONTROL ----------------
SEND_INTERVAL = 10  # seconds
last_send_time = time.time()

while True:
    ret, frame = cap.read()
    if not ret:
        break

    results = model(frame, conf=0.4)

    inventory_count = 0

    # ---------------- DETECTION LOOP ----------------
    for result in results:
        boxes = result.boxes
        for box in boxes:
            cls_id = int(box.cls[0])
            label = model.names[cls_id]
            confidence = float(box.conf[0])

            # Confidence filter
            if confidence < 0.4:
                continue

            # ICE CREAM LOGIC
            if label in ICE_CREAM_ALIASES:
                inventory_count += 1

    # ---------------- MQTT SEND (SLOW) ----------------
    current_time = time.time()
    if current_time - last_send_time >= SEND_INTERVAL:
        client.publish(MQTT_TOPIC, inventory_count)
        print("Inventory sent:", inventory_count)
        last_send_time = current_time

    # ---------------- DISPLAY ----------------
    annotated_frame = results[0].plot()

    cv2.putText(
        annotated_frame,
        f"Ice-Cream Inventory: {inventory_count}",
        (20, 40),
        cv2.FONT_HERSHEY_SIMPLEX,
        1,
        (0, 255, 0),
        2
    )

    cv2.imshow("YOLO Freezer Inventory", annotated_frame)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()
