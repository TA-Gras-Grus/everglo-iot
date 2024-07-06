#include <ESP8266WiFi.h> // Include this for ESP8266 boards
#include <PubSubClient.h>
#include <WebSocketsClient.h>  


// Replace with your network credentials
const char* ssid = "Jembut";
const char* password = "katsugosong";

// Replace with your MQTT Broker credentials
const char* mqtt_server = "02fa3ba5f83a4760bc66879c7e081d28.s1.eu.hivemq.cloud"; // Example broker
const int mqtt_port = 8884;
const char* mqtt_user = "everglo";
const char* mqtt_password = "everglo2024";

WiFiClientSecure espClient;
WebSocketsClient wsClient(espClient, mqtt_server, mqtt_port);
WebSocketsStreamClient wsStreamClient(wsClient, "mqtt");
PubSubClient client(wsStreamClient);

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
  Serial.printf("[WSc] Connected to url: %s\n", payload);
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client", mqtt_user, mqtt_password )) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("greenhouse:updated");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  // client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
