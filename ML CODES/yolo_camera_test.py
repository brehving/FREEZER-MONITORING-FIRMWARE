from ultralytics import YOLO
import cv2

model = YOLO("yolov8n.pt")
cap = cv2.VideoCapture(0)

ICECREAM_CLASSES = ["cup", "bottle", "box"]

while True:
    ret, frame = cap.read()
    if not ret:
        break

    results = model(frame, conf=0.5)
    detections = results[0].boxes

    icecream_count = 0

    for box in detections:
        cls_id = int(box.cls[0])
        label = model.names[cls_id]

        if label in ICECREAM_CLASSES:
            icecream_count += 1

    annotated = results[0].plot()
    cv2.putText(
        annotated,
        f"Ice-Cream Count: {icecream_count}",
        (20, 40),
        cv2.FONT_HERSHEY_SIMPLEX,
        1,
        (0, 255, 0),
        2
    )

    cv2.imshow("YOLO Inventory Counter", annotated)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()
