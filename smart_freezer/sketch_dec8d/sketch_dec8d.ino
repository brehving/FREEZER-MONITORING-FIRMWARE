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
#define DHTPIN 14
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

String urlUpload    = String("http://") + NODE_RED_HOST + ":" + NODE_RED_PORT + "/uploadData";
String urlAlert     = String("http://") + NODE_RED_HOST + ":" + NODE_RED_PORT + "/alert";

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
  Serial.print("Connecting WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nWiFi connected: " + WiFi.localIP().toString());
}

// ------------ MQTT CONNECT ----------
void connectMQTT() {
  while (!mqttClient.connected()) {
    Serial.print("Connecting to MQTT...");
    if (mqttClient.connect("esp32_freezer_device")) {
      Serial.println("connected.");
    } else {
      Serial.print("failed: ");
      Serial.println(mqttClient.state());
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
  int raw = digitalRead(DOOR_PIN);
  return (raw == LOW) ? 0 : 1; 
}

// ------------ SMOOTH VIBRATION (0–10) ----------
int readVibration() {
  static int vibLevel = 0; 
  static unsigned long lastDecay = 0;

  int v = digitalRead(VIB_PIN);  

  if (v == LOW) {   
    vibLevel += 3;  
    if (vibLevel > 10) vibLevel = 10;
  }

  unsigned long now = millis();
  if (now - lastDecay > 200) {  
    vibLevel -= 1;
    if (vibLevel < 0) vibLevel = 0;
    lastDecay = now;
  }

  return vibLevel;
}

// ------------ READ ALL SENSORS ----------
void readSensors(SensorData &d) {
  d.temp        = readTemperature();
  d.humidity    = readHumidity();
  d.door        = readDoor();
  d.vibration   = readVibration();   
  d.power       = random(100, 2000);
  d.highPressure= random(180, 360);
  d.lowPressure = random(10, 80);
  d.inventory   = random(0, 15);
}

// ------------ ALERT LOGIC ----------
String buildAlert(const SensorData &d) {
  if (d.temp > -10) return "HIGH TEMP ALERT!";
  if (d.temp < -30) return "LOW TEMP ALERT!";
  if (d.door == 1)  return "DOOR OPEN!";
  if (d.vibration > 7) return "HIGH VIBRATION!";
  if (d.power > 1500) return "POWER OVERLOAD!";
  return "SYSTEM NORMAL";
}

// ------------ HTTP POST ----------
void httpPost(String url, String body, const char* tag) {
  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;
  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  int code = http.POST(body);

  Serial.print(tag);
  Serial.print(" → HTTP ");
  Serial.println(code);

  http.end();
}

void uploadData(const SensorData &d) {
  String json = "{";
  json += "\"temp\":" + String(d.temp) + ",";
  json += "\"humidity\":" + String(d.humidity) + ",";
  json += "\"door\":" + String(d.door) + ",";
  json += "\"vibration\":" + String(d.vibration) + ",";
  json += "\"power\":" + String(d.power) + ",";
  json += "\"high_pressure\":" + String(d.highPressure) + ",";
  json += "\"low_pressure\":" + String(d.lowPressure) + ",";
  json += "\"inventory\":" + String(d.inventory);
  json += "}";

  httpPost(urlUpload, json, "[UPLOAD]");
}

void sendAlert(String alert, const SensorData &d) {
  String json = "{";
  json += "\"alert\":\"" + alert + "\",";
  json += "\"temp\":" + String(d.temp) + ",";
  json += "\"door\":" + String(d.door);
  json += "}";

  httpPost(urlAlert, json, "[ALERT]");
}

// ------------ MQTT PUBLISH ----------
void mqttPublish(const SensorData &d) {
  mqttClient.publish("freezer/temp", String(d.temp).c_str());
  mqttClient.publish("freezer/humidity", String(d.humidity).c_str());
  mqttClient.publish("freezer/door", String(d.door).c_str());
  mqttClient.publish("freezer/vibration", String(d.vibration).c_str());
  mqttClient.publish("freezer/power", String(d.power).c_str());
  mqttClient.publish("freezer/high_pressure", String(d.highPressure).c_str());
  mqttClient.publish("freezer/low_pressure", String(d.lowPressure).c_str());
  mqttClient.publish("freezer/inventory", String(d.inventory).c_str());
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
    sendAlert(alert, s);

    Serial.println("--------- CYCLE ---------");
    Serial.println("Temp: " + String(s.temp));
    Serial.println("Humidity: " + String(s.humidity));
    Serial.println("Door: " + String(s.door));
    Serial.println("Vibration: " + String(s.vibration));
    Serial.println("Power: " + String(s.power));
    Serial.println("High P: " + String(s.highPressure));
    Serial.println("Low P: " + String(s.lowPressure));
    Serial.println("Inventory: " + String(s.inventory));
    Serial.println("Alert: " + alert);
    Serial.println("--------------------------");
  }
}
