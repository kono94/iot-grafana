#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define MQ2PIN A0

#define MQTT_TOPIC_GAS "home/mq2/gas"
#define MQTT_TOPIC_STATE "home/mq2/status"
#define MQTT_PUBLISH_DELAY 5000
#define MQTT_CLIENT_ID "esp8266mq2"

const char* WIFI_SSID     = "";
const char* WIFI_PASSWORD = "";

const char *MQTT_SERVER = "192.168.0.12";
const char *MQTT_USER = NULL; // NULL for no authentication
const char *MQTT_PASSWORD = NULL; // NULL for no authentication


long lastMsgTime = 0;

WiFiClient espClient;
PubSubClient mqttClient(espClient);

void setup(){
    pinMode(MQ2PIN, INPUT);
    Serial.begin(115200);
    setupWifi();
    mqttClient.setServer(MQTT_SERVER, 1883);
}

void loop(){
    if (!mqttClient.connected()) {
        mqttReconnect();
    }
    mqttClient.loop();
    long now = millis();
    if (now - lastMsgTime > MQTT_PUBLISH_DELAY) {
        lastMsgTime = now;
        int gasValue = analogRead(MQ2PIN);
        delay(3);
        Serial.print("Pin A0: ");
        Serial.println(gasValue);
        // Publishing sensor data
        mqttPublish(MQTT_TOPIC_GAS, gasValue);
   }
}


void setupWifi() {
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void mqttPublish(char* topic, float payload) {
  Serial.print(topic);
  Serial.print(": ");
  Serial.println(payload);

  mqttClient.publish(topic, String(payload).c_str(), true);
}


void mqttReconnect() {
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");

    // Attempt to connect
    if (mqttClient.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASSWORD, MQTT_TOPIC_STATE, 1, true, "disconnected", false)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}
