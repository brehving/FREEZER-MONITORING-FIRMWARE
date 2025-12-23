#include "esp_camera.h"

// FLASH LED pin on ESP32-CAM
#define FLASH_LED_PIN 4

void setup() {
  Serial.begin(115200);
  delay(2000);

  pinMode(FLASH_LED_PIN, OUTPUT);

  Serial.println("ESP32-CAM TEST STARTED");
  Serial.println("If you see this, upload & serial is working");
}

void loop() {
  Serial.println("ESP32-CAM IS ALIVE");

  digitalWrite(FLASH_LED_PIN, HIGH); // Flash ON
  delay(500);

  digitalWrite(FLASH_LED_PIN, LOW);  // Flash OFF
  delay(500);
}
