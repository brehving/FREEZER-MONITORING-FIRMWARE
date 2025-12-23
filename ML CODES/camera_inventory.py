import cv2
import time
import random
import paho.mqtt.client as mqtt

# ---------------- MQTT CONFIG ----------------
MQTT_BROKER = "broker.hivemq.com"
MQTT_TOPIC  = "freezer/inventory"

client = mqtt.Client()
client.connect(MQTT_BROKER, 1883, 60)

# ---------------- CAMERA ----------------
cap = cv2.VideoCapture(0)

if not cap.isOpened():
    print("Camera not detected")
    exit()

print("Camera + Inventory system started")

# ---------------- TIMING ----------------
SEND_INTERVAL = 10  # seconds
last_update_time = time.time()

# ---------------- INVENTORY ----------------
inventory_count = random.randint(6, 10)

while True:
    ret, frame = cap.read()
    if not ret:
        break

    current_time = time.time()

    # Update + publish TOGETHER
    if current_time - last_update_time >= SEND_INTERVAL:
        change = random.choice([-1, 0, 0, 0])
        inventory_count = max(0, inventory_count + change)

        client.publish(MQTT_TOPIC, inventory_count)
        print("Inventory sent:", inventory_count)

        last_update_time = current_time

    # Display SAME value as MQTT
    cv2.putText(
        frame,
        f"Inventory: {inventory_count}",
        (20, 40),
        cv2.FONT_HERSHEY_SIMPLEX,
        1,
        (0, 255, 0),
        2
    )

    cv2.imshow("Ice-Cream Inventory Camera", frame)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()