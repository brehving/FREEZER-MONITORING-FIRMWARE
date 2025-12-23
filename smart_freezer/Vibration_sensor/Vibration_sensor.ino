#define VIB_PIN 26   // SW-420 DO pin

void setup() {
  Serial.begin(115200);
  pinMode(VIB_PIN, INPUT);
  Serial.println("SW-420 Reversed Logic Test");
}

void loop() {
  int v = digitalRead(VIB_PIN);

  if (v == LOW) {
    Serial.println("VIBRATION DETECTED");
  } else {
    Serial.println("NO VIBRATION");
  }

  delay(200);
}
