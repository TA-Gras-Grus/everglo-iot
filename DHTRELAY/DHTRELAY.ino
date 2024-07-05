#include <DHT.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h> // Library for TLS/SSL support
#include <PubSubClient.h>

// DHT Sensor setup
#define DHTPIN D2 // Pin connected to DHT11 data pin
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Relay pins
#define RELAY_PIN_HEATER D3
#define RELAY_PIN_FAN D4

// Wi-Fi credentials
const char* ssid = "Tongkrongan Membiru";
const char* password = "ngisorudel";

// MQTT server
const char* mqtt_server = "02fa3ba5f83a4760bc66879c7e081d28.s1.eu.hivemq.cloud";
const int mqtt_port = 8883; // Port for TLS connection
const char* mqtt_username = "everglo";
const char* mqtt_password = "everglo2024";
const char* mqtt_topic_greenhouse_update = "greenhouse:updated";
const char* mqtt_topic_greenhouse_data = "greenhouseData:created";

WiFiClientSecure espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);
  dht.begin();
  
  pinMode(RELAY_PIN_HEATER, OUTPUT);
  pinMode(RELAY_PIN_FAN, OUTPUT);
  digitalWrite(RELAY_PIN_HEATER, LOW);
  digitalWrite(RELAY_PIN_FAN, LOW);

  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  espClient.setInsecure();
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  if (String(topic) == mqtt_topic_greenhouse_update) {
    if ((char)payload[0] == '1') {
      digitalWrite(RELAY_PIN_HEATER, HIGH);
    } else if ((char)payload[0] == '0') {
      digitalWrite(RELAY_PIN_HEATER, LOW);
    }
    if ((char)payload[1] == '1') {
      digitalWrite(RELAY_PIN_FAN, HIGH);
    } else if ((char)payload[1] == '0') {
      digitalWrite(RELAY_PIN_FAN, LOW);
    }

    // Publish update to greenhouse:updated topic
    client.publish(mqtt_topic_greenhouse_update, payload, length);
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ArduinoClient", mqtt_username, mqtt_password)) {
      Serial.println("connected");
      client.subscribe(mqtt_topic_greenhouse_update);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  char tempString[8];
  dtostrf(t, 1, 2, tempString);

  char humString[8];
  dtostrf(h, 1, 2, humString);

  // Publish greenhouse data
  char data[32];
  snprintf(data, sizeof(data), "Temperature:%s,Humidity:%s", tempString, humString);
  client.publish(mqtt_topic_greenhouse_data, data);

  delay(2000);
}
