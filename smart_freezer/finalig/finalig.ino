#include <WiFi.h>
#include <HTTPClient.h>
#include <PubSubClient.h>

// ------------ DS18B20 -------------
#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 4 

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature ds18b20(&oneWire);

// ------------ DHT22 ----------------
#include "DHT.h"
#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// ------------ DOOR SWITCH ----------
#define DOOR_PIN 27   

// ------------ VIBRATION (SW-420) ----
#define VIB_PIN 26    

// ------------ WIFI CONFIG ----------
const char* WIFI_SSID     = "Ganesan_2.4G";
const char* WIFI_PASSWORD = "vkarthick";

// ------------ NODE-RED HTTP ---------
const char* NODE_RED_HOST = "192.168.1.1";  
const int NODE_RED_PORT   = 1880;

String urlUpload = String("http://") + NODE_RED_HOST + ":" + NODE_RED_PORT + "/uploadData";
String urlAlert  = String("http://") + NODE_RED_HOST + ":" + NODE_RED_PORT + "/alert";

// ----------- MQTT CONFIG ------------
const char* MQTT_SERVER = "broker.hivemq.com";
const int   MQTT_PORT   = 1883;

WiFiClient espClient;
PubSubClient mqttClient(espClient);

// ------------ SENSOR STRUCT ---------
struct SensorData {
  float temp;           
  float humidity;        
  int door;             
  int vibration;        
  int power;             
  int highPressure;     
  int lowPressure;      
  int inventory;        
};

SensorData s;

// ------------ WIFI CONNECT ----------
void connectWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}

// ------------ MQTT CONNECT ----------
void connectMQTT() {
  while (!mqttClient.connected()) {
    if (mqttClient.connect("ESP32_FREEZER")) {
      // connected
    } else {
      delay(2000);
    }
  }
}

// ------------ REAL TEMP ----------
float readTemperature() {
  ds18b20.requestTemperatures();
  float t = ds18b20.getTempCByIndex(0);
  if (t == DEVICE_DISCONNECTED_C) return 0;
  return t;
}

// ------------ REAL HUMIDITY ----------
float readHumidity() {
  float h = dht.readHumidity();
  if (isnan(h)) return 0;
  return h;
}

// ------------ REAL DOOR ----------
int readDoor() {
  return digitalRead(DOOR_PIN) == LOW ? 1 : 0;
}

// ------------ SMOOTH VIBRATION ----------
int readVibration() {
  static int vibLevel = 0; 
  static unsigned long lastDecay = 0;

  if (digitalRead(VIB_PIN) == LOW) {
    vibLevel += 3;
    if (vibLevel > 10) vibLevel = 10;
  }

  if (millis() - lastDecay > 200) {
    vibLevel--;
    if (vibLevel < 0) vibLevel = 0;
    lastDecay = millis();
  }

  return vibLevel;
}

// ------------ READ ALL SENSORS ----------
void readSensors(SensorData &d) {
  d.temp         = readTemperature();
  d.humidity     = readHumidity();
  d.door         = readDoor();
  d.vibration    = readVibration();
  d.power        = random(200, 1800);
  d.highPressure = random(200, 350);
  d.lowPressure  = random(20, 90);
  d.inventory    = random(0, 15);
}

// ------------ ALERT LOGIC (FREEZER RULES) ----------
String buildAlert(const SensorData &d) {

  if (d.temp > -15 && d.temp != 0)
    return "HIGH TEMPERATURE ALERT";

  if (d.temp < -35 && d.temp != 0)
    return "COOLING / SENSOR FAILURE";

  if (d.door == 1)
    return "DOOR OPEN ALERT";

  if (d.vibration > 7)
    return "HIGH VIBRATION ALERT";

  if (d.power > 1500)
    return "POWER OVERLOAD ALERT";

  return "NORMAL";
}

// ------------ HTTP POST ----------
void httpPost(String url, String body) {
  if (WiFi.status() != WL_CONNECTED) return;
  HTTPClient http;
  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  http.POST(body);
  http.end();
}

void uploadData(const SensorData &d) {
  String json =
    "{"
    "\"temp\":" + String(d.temp) + "," +
    "\"humidity\":" + String(d.humidity) + "," +
    "\"door\":" + String(d.door) + "," +
    "\"vibration\":" + String(d.vibration) + "," +
    "\"power\":" + String(d.power) + "," +
    "\"high_pressure\":" + String(d.highPressure) + "," +
    "\"low_pressure\":" + String(d.lowPressure) + "," +
    "\"inventory\":" + String(d.inventory) +
    "}";

  httpPost(urlUpload, json);
}

void sendAlert(String alert, const SensorData &d) {
  String json =
    "{"
    "\"alert\":\"" + alert + "\"," +
    "\"temp\":" + String(d.temp) + "," +
    "\"door\":" + String(d.door) +
    "}";

  httpPost(urlAlert, json);
}

// ------------ MQTT PUBLISH ----------
void mqttPublish(const SensorData &d) {
  mqttClient.publish("freezer/temp", String(d.temp).c_str());
  mqttClient.publish("freezer/humidity", String(d.humidity).c_str());
  mqttClient.publish("freezer/door", String(d.door).c_str());
  mqttClient.publish("freezer/vibration", String(d.vibration).c_str());
  mqttClient.publish("freezer/power", String(d.power).c_str());
}

// ------------ LOOP TIMING ----------
unsigned long lastSend = 0;
const unsigned long SEND_INTERVAL = 5000;

// ------------ SETUP ----------
void setup() {
  Serial.begin(115200);

  ds18b20.begin();
  dht.begin();

  pinMode(DOOR_PIN, INPUT_PULLUP);
  pinMode(VIB_PIN, INPUT);

  connectWiFi();
  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  connectMQTT();

  randomSeed(analogRead(34));
}

// ------------ MAIN LOOP ----------
void loop() {
  if (WiFi.status() != WL_CONNECTED) connectWiFi();
  if (!mqttClient.connected()) connectMQTT();
  mqttClient.loop();

  if (millis() - lastSend >= SEND_INTERVAL) {
    lastSend = millis();

    readSensors(s);
    String alert = buildAlert(s);

    mqttPublish(s);
    uploadData(s);

    if (alert != "NORMAL") {
      sendAlert(alert, s);
    }

    Serial.println("------ FREEZER STATUS ------");
    Serial.println("Temp: " + String(s.temp));
    Serial.println("Humidity: " + String(s.humidity));
    Serial.println("Door: " + String(s.door));
    Serial.println("Vibration: " + String(s.vibration));
    Serial.println("Power: " + String(s.power));
    Serial.println("Alert: " + alert);
    Serial.println("----------------------------");
  }
}