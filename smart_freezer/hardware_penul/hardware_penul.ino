#include <WiFi.h>
#include <HTTPClient.h>
#include <PubSubClient.h>

// ========= DS18B20 =========
#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 4
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature ds18b20(&oneWire);

// ========= DHT22 =========
#include "DHT.h"
#define DHTPIN 14
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// ========= DOOR (REED) =========
#define DOOR_PIN 27   // reed switch between GPIO27 and GND

// ========= VIBRATION (SW-420) =========
#define VIB_PIN 26    // SW-420 DO pin

// ========= INA219 (CURRENT + POWER) =========
#include <Wire.h>
#include <Adafruit_INA219.h>
Adafruit_INA219 ina219;

// ========= WIFI CONFIG =========
const char* WIFI_SSID     = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// ========= NODE-RED HTTP CONFIG =========
const char* NODE_RED_HOST = "192.168.1.5";   // <- change to your PC IP
const int   NODE_RED_PORT = 1880;

String urlUpload =   String("http://") + NODE_RED_HOST + ":" + NODE_RED_PORT + "/uploadData";
String urlAlert  =   String("http://") + NODE_RED_HOST + ":" + NODE_RED_PORT + "/alert";

// ========= MQTT CONFIG =========
const char* MQTT_SERVER = "broker.hivemq.com";
const int   MQTT_PORT   = 1883;
WiFiClient espClient;
PubSubClient mqttClient(espClient);

// ========= DATA STRUCT =========
struct SensorData {
  float temp;        // °C  (DS18B20)
  float humidity;    // %   (DHT22)
  int   door;        // 0 closed, 1 open
  int   vibration;   // 0–10
  float current;     // A   (INA219)
  float power;       // W   (INA219)
  int   highPressure; // fake
  int   lowPressure;  // fake
  int   inventory;    // fake
};

SensorData s;

// ========= WIFI =========
void connectWiFi() {
  Serial.print("Connecting WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nWiFi connected: " + WiFi.localIP().toString());
}

// ========= MQTT =========
void connectMQTT() {
  while (!mqttClient.connected()) {
    Serial.print("Connecting MQTT...");
    if (mqttClient.connect("esp32_freezer_device")) {
      Serial.println("connected.");
    } else {
      Serial.print("failed, state=");
      Serial.println(mqttClient.state());
      delay(2000);
    }
  }
}

// ========= REAL TEMPERATURE =========
float readTemperature() {
  ds18b20.requestTemperatures();
  float t = ds18b20.getTempCByIndex(0);
  if (t == DEVICE_DISCONNECTED_C) return 0;
  return t;
}

// ========= REAL HUMIDITY =========
float readHumidity() {
  float h = dht.readHumidity();
  if (isnan(h)) return 0;
  return h;
}

// ========= REAL DOOR =========
int readDoor() {
  int raw = digitalRead(DOOR_PIN);
  return (raw == LOW) ? 0 : 1;  // 0 closed, 1 open
}

// ========= SMOOTH VIBRATION 0–10 =========
// SW-420: LOW = vibration detected (your board)
int readVibration() {
  static int vibLevel = 0; 
  static unsigned long lastDecay = 0;

  int v = digitalRead(VIB_PIN);  

  if (v == LOW) {                 // vibration hit
    vibLevel += 3;
    if (vibLevel > 10) vibLevel = 10;
  }

  unsigned long now = millis();
  if (now - lastDecay > 200) {    // decay every 200 ms
    vibLevel -= 1;
    if (vibLevel < 0) vibLevel = 0;
    lastDecay = now;
  }

  return vibLevel;
}

// ========= REAL CURRENT + POWER (INA219) =========
void readPowerAndCurrent(SensorData &d) {
  // If INA219 not found earlier, this will just return 0s
  float busV      = ina219.getBusVoltage_V();   // not used right now, but available
  float current_mA= ina219.getCurrent_mA();
  float power_mW  = ina219.getPower_mW();

  // Protect against NAN
  if (isnan(current_mA) || isnan(power_mW)) {
    d.current = 0;
    d.power   = 0;
  } else {
    d.current = current_mA / 1000.0f;   // A
    d.power   = power_mW   / 1000.0f;   // W
  }
}

// ========= READ ALL SENSORS =========
void readSensors(SensorData &d) {
  d.temp      = readTemperature();
  d.humidity  = readHumidity();
  d.door      = readDoor();
  d.vibration = readVibration();
  readPowerAndCurrent(d);              // fills current + power

  d.highPressure = random(180, 360);
  d.lowPressure  = random(10, 80);
  d.inventory    = random(0, 15);
}

// ========= ALERT LOGIC =========
String buildAlert(const SensorData &d) {
  if (d.temp > -10)   return "HIGH TEMP ALERT!";
  if (d.temp < -30)   return "LOW TEMP ALERT!";
  if (d.door == 1)    return "DOOR OPEN!";
  if (d.vibration > 7) return "HIGH VIBRATION!";
  if (d.power > 1500) return "POWER OVERLOAD!";    // threshold in W (for real freezer later)
  return "SYSTEM NORMAL";
}

// ========= HTTP HELPERS =========
void httpPost(const String &url, const String &body, const char* tag) {
  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;
  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  int code = http.POST(body);

  Serial.print(tag);
  Serial.print(" -> HTTP ");
  Serial.println(code);

  http.end();
}

void uploadData(const SensorData &d) {
  String json = "{";
  json += "\"temp\":"      + String(d.temp)      + ",";
  json += "\"humidity\":"  + String(d.humidity)  + ",";
  json += "\"door\":"      + String(d.door)      + ",";
  json += "\"vibration\":" + String(d.vibration) + ",";
  json += "\"current\":"   + String(d.current, 3)+ ",";  // A
  json += "\"power\":"     + String(d.power, 2)  + ",";  // W
  json += "\"high_pressure\":" + String(d.highPressure) + ",";
  json += "\"low_pressure\":"  + String(d.lowPressure)  + ",";
  json += "\"inventory\":"     + String(d.inventory);
  json += "}";

  httpPost(urlUpload, json, "[UPLOAD]");
}

void sendAlert(const String &alert, const SensorData &d) {
  String json = "{";
  json += "\"alert\":\"" + alert + "\",";
  json += "\"temp\":"    + String(d.temp) + ",";
  json += "\"door\":"    + String(d.door) + ",";
  json += "\"power\":"   + String(d.power, 2);
  json += "}";

  httpPost(urlAlert, json, "[ALERT]");
}

// ========= MQTT PUBLISH =========
void mqttPublish(const SensorData &d) {
  mqttClient.publish("freezer/temp",        String(d.temp).c_str());
  mqttClient.publish("freezer/humidity",    String(d.humidity).c_str());
  mqttClient.publish("freezer/door",        String(d.door).c_str());
  mqttClient.publish("freezer/vibration",   String(d.vibration).c_str());
  mqttClient.publish("freezer/current",     String(d.current, 3).c_str());
  mqttClient.publish("freezer/power",       String(d.power, 2).c_str());
  mqttClient.publish("freezer/high_pressure", String(d.highPressure).c_str());
  mqttClient.publish("freezer/low_pressure",  String(d.lowPressure).c_str());
  mqttClient.publish("freezer/inventory",     String(d.inventory).c_str());
}

// ========= LOOP TIMING =========
unsigned long lastSend = 0;
const unsigned long SEND_INTERVAL = 5000;

// ========= SETUP =========
void setup() {
  Serial.begin(115200);

  ds18b20.begin();
  dht.begin();
  pinMode(DOOR_PIN, INPUT_PULLUP);
  pinMode(VIB_PIN, INPUT);

  // I2C for INA219
  Wire.begin(21, 22);
  if (!ina219.begin()) {
    Serial.println("INA219 NOT FOUND, power/current will be 0!");
  } else {
    ina219.setCalibration_32V_2A();   // default good calibration
  }

  connectWiFi();

  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  connectMQTT();

  randomSeed(analogRead(34));
}

// ========= LOOP =========
void loop() {
  if (WiFi.status() != WL_CONNECTED) connectWiFi();
  if (!mqttClient.connected())       connectMQTT();
  mqttClient.loop();

  if (millis() - lastSend >= SEND_INTERVAL) {
    lastSend = millis();

    readSensors(s);
    String alert = buildAlert(s);

    mqttPublish(s);
    uploadData(s);
    sendAlert(alert, s);

    Serial.println("--------- CYCLE ---------");
    Serial.println("Temp:       " + String(s.temp));
    Serial.println("Humidity:   " + String(s.humidity));
    Serial.println("Door:       " + String(s.door));
    Serial.println("Vibration:  " + String(s.vibration));
    Serial.println("Current:    " + String(s.current, 3) + " A");
    Serial.println("Power:      " + String(s.power, 2)   + " W");
    Serial.println("High P:     " + String(s.highPressure));
    Serial.println("Low P:      " + String(s.lowPressure));
    Serial.println("Inventory:  " + String(s.inventory));
    Serial.println("Alert:      " + alert);
    Serial.println("-------------------------");
  }
}
