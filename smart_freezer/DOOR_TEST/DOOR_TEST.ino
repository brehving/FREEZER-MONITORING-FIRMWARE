#define DOOR_PIN 27  // Reed switch wire goes here (other wire to GND)

void setup() {
  Serial.begin(115200);

  // Internal pull-up means:
  // - When switch is CLOSED -> reads LOW
  // - When switch is OPEN  -> reads HIGH
  pinMode(DOOR_PIN, INPUT_PULLUP);

  Serial.println("Reed Switch Door Sensor Test Started");
}

void loop() {
  int doorState = digitalRead(DOOR_PIN);

  if (doorState == LOW) {
    Serial.println("DOOR CLOSED");
  } else {
    Serial.println("DOOR OPEN");
  }

  delay(300);
}
