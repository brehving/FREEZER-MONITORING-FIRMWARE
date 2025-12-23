#include <WiFi.h>
#include <HTTPClient.h>
#include <PubSubClient.h>

// ------------ DS18B20 -------------
#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 4  // DS18B20 data pin on GPIO 4

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature ds18b20(&oneWire);

// ------------ DHT22 ----------------
#include "DHT.h"
#define DHTPIN 14       // DHT22 DATA on GPIO 14
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);

// ------------ REED DOOR SWITCH -----
#define DOOR_PIN 27     // one wire to GPIO 27, other wire to GND

// -------------------------------------------------
// 1) WIFI CONFIG
// -------------------------------------------------
const char* WIFI_SSID     = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// -------------------------------------------------
// 2) NODE-RED / HTTP API CONFIG
// -------------------------------------------------
const char* NODE_RED_HOST = "192.168.1.5";   // CHANGE THIS TO YOUR LAPTOP IP
const int   NODE_RED_PORT = 1880;

String urlUpload    = String("http://") + NODE_RED_HOST + ":" + NODE_RED_PORT + "/uploadData";
String urlAlert     = String("http://") + NODE_RED_HOST + ":" + NODE_RED_PORT + "/alert";
String urlInventory = String("http://") + NODE_RED_HOST + ":" + NODE_RED_PORT + "/inventory";

// -------------------------------------------------
// 3) MQTT CONFIG
// -------------------------------------------------
const char* MQTT_SERVER = "broker.hivemq.com";
const int   MQTT_PORT   = 1883;

WiFiClient espClient;
PubSubClient mqttClient(espClient);

// -------------------------------------------------
struct SensorData {
  float temp;          // REAL temp (DS18B20)
  float humidity;      // REAL humidity (DHT22)
  int   vibration;     // random
  int   door;          // REAL: 0 = closed, 1 = open
  int   power;         // random
  int   highPressure;  // random
  int   lowPressure;   // random
  int   inventory;     // random
};
SensorData s;

// -------------------------------------------------
// WIFI
// -------------------------------------------------
void connectWiFi() {
  Serial.print("Connecting to WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected.");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

// -------------------------------------------------
// MQTT
// -------------------------------------------------
void connectMQTT() {
  while (!mqttClient.connected()) {
    Serial.print("Connecting to MQTT...");
    if (mqttClient.connect("esp32_freezer")) {
      Serial.println("connected.");
    } else {
      Serial.print("failed: ");
      Serial.println(mqttClient.state());
      delay(2000);
    }
  }
}

// -------------------------------------------------
// REAL TEMP FROM DS18B20
// -------------------------------------------------
float readRealTemp() {
  ds18b20.requestTemperatures();
  float t = ds18b20.getTempCByIndex(0);

  if (t == DEVICE_DISCONNECTED_C) {
    Serial.println("ERROR: DS18B20 not found!");
    return 0;
  }

  Serial.print("DS18B20 Temp: ");
  Serial.println(t);

  return t;
}

// -------------------------------------------------
// REAL HUMIDITY FROM DHT22
// -------------------------------------------------
float readRealHumidity() {
  float h = dht.readHumidity();

  if (isnan(h)) {
    Serial.println("ERROR: DHT22 humidity error!");
    return 0;
  }

  Serial.print("DHT22 Humidity: ");
  Serial.println(h);

  return h;
}

// -------------------------------------------------
// REAL DOOR FROM REED SWITCH
// -------------------------------------------------
// INPUT_PULLUP wiring: GPIO27 --- reed --- GND
// raw LOW  -> switch closed -> DOOR CLOSED (0)
// raw HIGH -> switch open   -> DOOR OPEN   (1)
int readDoorState() {
  int raw = digitalRead(DOOR_PIN);
  int door = (raw == LOW) ? 0 : 1;    // 0 = closed, 1 = open

  if (door == 0) {
    Serial.println("Door sensor: CLOSED");
  } else {
    Serial.println("Door sensor: OPEN");
  }

  return door;
}

// -------------------------------------------------
// READ ALL SENSORS (REAL + RANDOM)
// -------------------------------------------------
void readSensors(SensorData &d) {
  d.temp        = readRealTemp();       // real temp
  d.humidity    = readRealHumidity();   // real humidity
  d.door        = readDoorState();      // real door
  d.vibration   = random(0, 11);        // fake for now
  d.power       = random(100, 2000);
  d.highPressure= random(180, 360);
  d.lowPressure = random(10, 80);
  d.inventory   = random(0, 15);
}

// -------------------------------------------------
// SIMPLE ALERT LOGIC
// -------------------------------------------------
String buildAlertText(const SensorData &d) {
  if (d.temp > -10)  return "HIGH TEMP ALERT!";
  if (d.temp < -30)  return "LOW TEMP ALERT!";
  if (d.door == 1)   return "DOOR OPEN! Close immediately.";
  if (d.vibration > 7) return "HIGH VIBRATION!";
  if (d.power > 1500)  return "POWER OVERLOAD!";
  return "SYSTEM NORMAL";
}

// -------------------------------------------------
// HTTP POST
// -------------------------------------------------
void httpPostJSON(String url, String body, const char* tag) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println(String(tag) + " WiFi not connected, skip");
    return;
  }

  HTTPClient http;
  http.begin(url);
  http.addHeader("Content-Type", "application/json");

  int code = http.POST(body);

  Serial.print(tag);
  Serial.print(" -> HTTP ");
  Serial.println(code);

  http.end();
}

void sendUploadData(const SensorData &d) {
  String json = "{";
  json += "\"temp\":" + String(d.temp) + ",";
  json += "\"humidity\":" + String(d.humidity) + ",";
  json += "\"vibration\":" + String(d.vibration) + ",";
  json += "\"door\":" + String(d.door) + ",";
  json += "\"power\":" + String(d.power) + ",";
  json += "\"high_pressure\":" + String(d.highPressure) + ",";
  json += "\"low_pressure\":" + String(d.lowPressure) + ",";
  json += "\"inventory\":" + String(d.inventory);
  json += "}";

  httpPostJSON(urlUpload, json, "[UPLOAD]");
}

void sendAlert(const String &alert, const SensorData &d) {
  String json = "{";
  json += "\"alert\":\"" + alert + "\",";
  json += "\"temp\":" + String(d.temp) + ",";
  json += "\"door\":" + String(d.door);
  json += "}";

  httpPostJSON(urlAlert, json, "[ALERT]");
}

// -------------------------------------------------
// MQTT
// -------------------------------------------------
void mqttPublish(const SensorData &d) {
  if (!mqttClient.connected()) return;

  mqttClient.publish("freezer/temp",          String(d.temp).c_str());
  mqttClient.publish("freezer/humidity",      String(d.humidity).c_str());
  mqttClient.publish("freezer/vibration",     String(d.vibration).c_str());
  mqttClient.publish("freezer/door",          String(d.door).c_str());
  mqttClient.publish("freezer/power",         String(d.power).c_str());
  mqttClient.publish("freezer/high_pressure", String(d.highPressure).c_str());
  mqttClient.publish("freezer/low_pressure",  String(d.lowPressure).c_str());
  mqttClient.publish("freezer/inventory",     String(d.inventory).c_str());

  Serial.println("MQTT Published.");
}

// -------------------------------------------------
unsigned long lastSend = 0;
const unsigned long SEND_INTERVAL = 5000;

// -------------------------------------------------
// SETUP
// -------------------------------------------------
void setup() {
  Serial.begin(115200);

  ds18b20.begin();
  dht.begin();
  pinMode(DOOR_PIN, INPUT_PULLUP);

  connectWiFi();
  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  connectMQTT();

  randomSeed(analogRead(34));
}

// -------------------------------------------------
// LOOP
// -------------------------------------------------
void loop() {
  if (WiFi.status() != WL_CONNECTED) connectWiFi();
  if (!mqttClient.connected()) connectMQTT();
  mqttClient.loop();

  unsigned long now = millis();
  if (now - lastSend >= SEND_INTERVAL) {
    lastSend = now;

    readSensors(s);
    String alert = buildAlertText(s);

    mqttPublish(s);
    sendUploadData(s);
    sendAlert(alert, s);

    Serial.println("---- CYCLE ----");
    Serial.println("Temp: " + String(s.temp));
    Serial.println("Humidity: " + String(s.humidity));
    Serial.println("Door: " + String(s.door)); // 0 closed, 1 open
    Serial.println("Vibration: " + String(s.vibration));
    Serial.println("Power: " + String(s.power));
    Serial.println("HP: " + String(s.highPressure));
    Serial.println("LP: " + String(s.lowPressure));
    Serial.println("Inventory: " + String(s.inventory));
    Serial.println("Alert: " + alert);
    Serial.println("------------------");
  }
}
