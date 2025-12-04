#include <WiFi.h>
#include <HTTPClient.h>
#include <PubSubClient.h>

// -------------------------------------------------
// 1) WIFI CONFIG
// -------------------------------------------------
const char* WIFI_SSID     = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// -------------------------------------------------
// 2) NODE-RED / HTTP API CONFIG
//    This is your laptop IP where Node-RED is running
//    Example: "192.168.1.5"
// -------------------------------------------------
const char* NODE_RED_HOST = "192.168.1.5";   // CHANGE THIS
const int   NODE_RED_PORT = 1880;            // default Node-RED port

String urlUpload   = String("http://") + NODE_RED_HOST + ":" + NODE_RED_PORT + "/uploadData";
String urlAlert    = String("http://") + NODE_RED_HOST + ":" + NODE_RED_PORT + "/alert";
String urlInventory= String("http://") + NODE_RED_HOST + ":" + NODE_RED_PORT + "/inventory";

// -------------------------------------------------
// 3) MQTT CONFIG (HiveMQ public broker)
// -------------------------------------------------
const char* MQTT_SERVER = "broker.hivemq.com";
const int   MQTT_PORT   = 1883;

WiFiClient espClient;
PubSubClient mqttClient(espClient);

// -------------------------------------------------
// 4) SENSOR STRUCT (FAKE SENSORS FOR NOW)
// -------------------------------------------------
struct SensorData {
  int   temp;          // °C
  int   humidity;      // %
  int   vibration;     // 0–10
  int   door;          // 0 closed, 1 open
  int   power;         // W
  int   highPressure;  // psi
  int   lowPressure;   // psi
  int   inventory;     // items
};

SensorData s;

// -------------------------------------------------
// WIFI HELPERS
// -------------------------------------------------
void connectWiFi() {
  Serial.print("Connecting to WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected. IP: " + WiFi.localIP().toString());
}

// -------------------------------------------------
// MQTT HELPERS
// -------------------------------------------------
void connectMQTT() {
  while (!mqttClient.connected()) {
    Serial.print("Connecting to MQTT...");
    if (mqttClient.connect("esp32_freezer_device")) {
      Serial.println("connected.");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 2 seconds");
      delay(2000);
    }
  }
}

// -------------------------------------------------
// FAKE SENSOR GENERATION
// (Replace with real sensor reads later)
// -------------------------------------------------
void readFakeSensors(SensorData &d) {
  d.temp        = random(-40, 21);   // -40 to +20
  d.humidity    = random(20, 90);    // 20–90%
  d.vibration   = random(0, 11);     // 0–10
  d.door        = random(0, 2);      // 0 or 1
  d.power       = random(100, 2000); // 100–2000 W
  d.highPressure= random(180, 360);  // 180–360 psi
  d.lowPressure = random(10, 80);    // 10–80 psi
  d.inventory   = random(0, 15);     // 0–14 items
}

// -------------------------------------------------
// ALERT LOGIC (just builds a simple text summary)
// -------------------------------------------------
String buildAlertText(const SensorData &d) {
  // Temp thresholds
  const int TEMP_HIGH = -10;
  const int TEMP_LOW  = -30;

  // We just check a few key sensors; you can expand
  if (d.temp > TEMP_HIGH) {
    return "HIGH TEMP: Freezer too warm!";
  } else if (d.temp < TEMP_LOW) {
    return "LOW TEMP: Freezer too cold!";
  }

  if (d.door == 1) {
    return "DOOR OPEN: Close immediately!";
  }

  if (d.vibration > 7) {
    return "EXTREME VIBRATION: Check compressor!";
  }

  if (d.power > 1500) {
    return "POWER ALERT: Overload!";
  }

  // Otherwise normal
  return "SYSTEM NORMAL: All readings within range.";
}

// -------------------------------------------------
// HTTP POST HELPERS
// -------------------------------------------------
void httpPostJSON(const String &url, const String &jsonBody, const char* tag) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println(String(tag) + " - WiFi not connected, skip HTTP POST");
    return;
  }

  HTTPClient http;
  http.begin(url);
  http.addHeader("Content-Type", "application/json");

  int httpCode = http.POST(jsonBody);

  Serial.print(tag);
  Serial.print(" - HTTP POST to ");
  Serial.print(url);
  Serial.print(" -> Code: ");
  Serial.println(httpCode);

  http.end();
}

// Send all sensor data to /uploadData
void sendUploadData(const SensorData &d) {
  // Simple JSON object – adjust keys to match your Node-RED if needed
  String json = "{";
  json += "\"temp\":"          + String(d.temp)        + ",";
  json += "\"humidity\":"      + String(d.humidity)    + ",";
  json += "\"vibration\":"     + String(d.vibration)   + ",";
  json += "\"door\":"          + String(d.door)        + ",";
  json += "\"power\":"         + String(d.power)       + ",";
  json += "\"high_pressure\":" + String(d.highPressure)+ ",";
  json += "\"low_pressure\":"  + String(d.lowPressure) + ",";
  json += "\"inventory\":"     + String(d.inventory);
  json += "}";

  httpPostJSON(urlUpload, json, "[UPLOAD]");
}

// Send alert text to /alert
void sendAlertToCloud(const String &alertText, const SensorData &d) {
  String json = "{";
  json += "\"alert\":\"" + alertText + "\",";
  json += "\"temp\":"    + String(d.temp) + ",";
  json += "\"door\":"    + String(d.door);
  json += "}";

  httpPostJSON(urlAlert, json, "[ALERT]");
}

// Send inventory info to /inventory
void sendInventoryToCloud(const SensorData &d) {
  // Dummy image URL for now
  String imageUrl = "http://example.com/freezer_snapshot.jpg";

  String json = "{";
  json += "\"count\":"     + String(d.inventory) + ",";
  json += "\"image_url\":\"" + imageUrl + "\"";
  json += "}";

  httpPostJSON(urlInventory, json, "[INVENTORY]");
}

// -------------------------------------------------
// MQTT PUBLISH OF ALL SENSOR VALUES
// -------------------------------------------------
void publishAllToMQTT(const SensorData &d) {
  if (!mqttClient.connected()) return;

  mqttClient.publish("freezer/temp",         String(d.temp).c_str());
  mqttClient.publish("freezer/humidity",     String(d.humidity).c_str());
  mqttClient.publish("freezer/vibration",    String(d.vibration).c_str());
  mqttClient.publish("freezer/door",         String(d.door).c_str());
  mqttClient.publish("freezer/power",        String(d.power).c_str());
  mqttClient.publish("freezer/high_pressure",String(d.highPressure).c_str());
  mqttClient.publish("freezer/low_pressure", String(d.lowPressure).c_str());
  mqttClient.publish("freezer/inventory",    String(d.inventory).c_str());

  Serial.println("MQTT: Published all sensor values");
}

// -------------------------------------------------
// SETUP
// -------------------------------------------------
void setup() {
  Serial.begin(115200);
  delay(1000);

  connectWiFi();

  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  connectMQTT();

  randomSeed(analogRead(0));  // seed RNG for fake sensors
}

// -------------------------------------------------
// LOOP
// -------------------------------------------------
unsigned long lastSend = 0;
const unsigned long SEND_INTERVAL = 5000; // 5 seconds

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }

  if (!mqttClient.connected()) {
    connectMQTT();
  }
  mqttClient.loop();

  unsigned long now = millis();
  if (now - lastSend >= SEND_INTERVAL) {
    lastSend = now;

    // 1) Read sensors (fake)
    readFakeSensors(s);

    // 2) Build alert text
    String alertText = buildAlertText(s);

    // 3) Publish to MQTT
    publishAllToMQTT(s);

    // 4) Send to Node-RED HTTP APIs
    sendUploadData(s);
    sendAlertToCloud(alertText, s);
    sendInventoryToCloud(s);

    // 5) Debug log
    Serial.println("---- Cycle ----");
    Serial.println("Temp: " + String(s.temp));
    Serial.println("Hum : " + String(s.humidity));
    Serial.println("Door: " + String(s.door));
    Serial.println("Vib : " + String(s.vibration));
    Serial.println("Pow : " + String(s.power));
    Serial.println("HP  : " + String(s.highPressure));
    Serial.println("LP  : " + String(s.lowPressure));
    Serial.println("Inv : " + String(s.inventory));
    Serial.println("Alert: " + alertText);
    Serial.println("------------------------");
  }
}