#include <ArduinoMqttClient.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include "arduino_secrets.h"

char ssid[] = SECRET_SSID; 
char pass[] = SECRET_PASS; 

WiFiClientSecure wifiClient;
MqttClient mqttClient(wifiClient);

const char broker[] = "a7701bd2b3e54353b8aeab74b9c7f322.s1.eu.hivemq.cloud";
int port = 8883;
const char topic[] = "greenhouse:updated";  // Subscribe to all topics
const char* greenhouse_key = "e0454bb7-5683-4a65-a237-e5952849e851";
const int relayPin1 = D3;  // GPIO0

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, pass);;

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  wifiClient.setInsecure();
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(115200);
  pinMode(relayPin1, OUTPUT);
  digitalWrite(relayPin1, LOW);  // Initialize relay as off
  while (!Serial) {
    ;  // wait for serial port to connect. Needed for native USB port only
  }
  setup_wifi();

  mqttClient.setId("clientId");

  mqttClient.setUsernamePassword("everglo", "Everglo2024");

  Serial.print("Attempting to connect to the MQTT broker: ");
  Serial.println(broker);

  if (!mqttClient.connect(broker, port)) {
    Serial.print("MQTT connection failed! Error code = ");
    Serial.println(mqttClient.connectError());

    while (1)
      ;
  }

  Serial.println("You're connected to the MQTT broker!");
  Serial.println();

  Serial.print("Subscribing to all topics");
  Serial.println();

  // subscribe to all topics
  mqttClient.subscribe(topic);

  // set the message receive callback
  mqttClient.onMessage(onMqttMessage);

  Serial.print("Waiting for messages on all topics");
  Serial.println();
}


void loop() {
  // call poll() regularly to allow the library to receive MQTT messages and
  // send MQTT keep alives which avoids being disconnected by the broker
  mqttClient.poll();
}

void onMqttMessage(int messageSize) {
  // we received a message, print out the topic and contents
  Serial.print("Received a message with topic '");
  Serial.print(mqttClient.messageTopic());
  Serial.print("', length ");
  Serial.print(messageSize);
  Serial.println(" bytes:");

  // Create a buffer to hold the incoming message
  char message[messageSize + 1];
  int index = 0;

  // Read the message into the buffer
  while (mqttClient.available()) {
    message[index++] = (char)mqttClient.read();
  }
  message[messageSize] = '\0';  // Null-terminate the buffer

  // Print the received message
  Serial.println("Message:");
  Serial.println(message);

  // Parse JSON
  StaticJsonDocument<1024> doc;
  DeserializationError error = deserializeJson(doc, message);

  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.f_str());
    return;
  }

  // Extract values
  const char* greenhouseId = doc["dripIrrigationDevice"];
  bool statusWaterFlow = doc["statusWaterFlow"];
  // Compare greenhouseId with greenhouse_key
  if (strcmp(greenhouseId, greenhouse_key) == 0) {

    //handle blower
    if (statusWaterFlow) {
      digitalWrite(relayPin1, LOW);  // Turn relay 1 off
    } else {
      digitalWrite(relayPin1, HIGH);  // Turn relay 1 on
    }
  }
}
