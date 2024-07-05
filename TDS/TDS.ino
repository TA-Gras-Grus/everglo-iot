#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <SoftwareSerial.h>
#include <ModbusMaster.h>

const char* ssid = "SSID";
const char* password = "PASSWORD";
const char* mqtt_server = "broker.hivemq.com";

//Dosing pump1 = D1 pump2 = d2
const int pump1Pin = D1;
const int pump2Pin = D2;

// // For software serial PHmeter
// const int rs485RxPin = D3;
// const int rs485TxPin = D4;

//Water level switches
const int waterLevel1Pin = D5;
const int waterLevel2Pin = D6;

//relay pin
const int relayPin = D7;


WiFiClient espClient;
PubSubClient client(espClient);
// SoftwareSerial rs485Serial(rs485RxPin, rs485TxPin);
ModbusMaster phMeter;

// Topics for MQTT
const char* ecTopic = "home/garden/ec";
const char* phTopic = "home/garden/ph";
const char* pumpControlTopic = "home/garden/pumpControl";
const char* relayControlTopic = "home/garden/relayControl";
const char* waterLevel1Topic = "home/garden/waterlevel1";
const char* waterLevel2Topic = "home/garden/waterlevel2";

void setup() {
  Serial.begin(9600);
  rs485Serial.begin(9600);  
  pinMode(pump1Pin, OUTPUT);
  pinMode(pump2Pin, OUTPUT);
  pinMode(relayPin, OUTPUT);
  pinMode(waterLevel1Pin, INPUT);
  pinMode(waterLevel2Pin, INPUT);

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  // Initialize Modbus 
  phMeter.begin(2, rs485Serial);  // Assumes the pH meter ataddress 2
}

void setup_wifi() {
  delay(10);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("WiFi connected");
}

void callback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  if (String(topic) == pumpControlTopic) {
    if (message == "pump1_on") {
      digitalWrite(pump1Pin, HIGH);
    } else if (message == "pump1_off") {
      digitalWrite(pump1Pin, LOW);
    } else if (message == "pump2_on") {
      digitalWrite(pump2Pin, HIGH);
    } else if (message == "pump2_off") {
      digitalWrite(pump2Pin, LOW);
    }
  }

  if (String(topic) == relayControlTopic) {
    if (message == "relay_on") {
      digitalWrite(relayPin, HIGH);
    } else if (message == "relay_off") {
      digitalWrite(relayPin, LOW);
    }
  }
}

void reconnect() {
  while (!client.connected()) {
    if (client.connect("WemosD1MiniClient")) {
      client.subscribe(pumpControlTopic);
      client.subscribe(relayControlTopic);
    } else {
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Read EC 
  int ecValueRaw = analogRead(A0);
  float ecValue = ecValueRaw * (5.0 / 1023.0);  // Convert raw value to voltage
  char ecStr[8];
  dtostrf(ecValue, 6, 2, ecStr);
  client.publish(ecTopic, ecStr);

  // // Read pH 
  // uint8_t resultPH = phMeter.readInputRegisters(0x00, 1);

  // if (resultPH == phMeter.ku8MBSuccess) {
  //   float phValue = phMeter.getResponseBuffer(0x00) / 100.0;
  //   char phStr[8];
  //   dtostrf(phValue, 6, 2, phStr);
  //   client.publish(phTopic, phStr);
  // }

  // Read water level switch
  int waterLevel1 = digitalRead(waterLevel1Pin);
  int waterLevel2 = digitalRead(waterLevel2Pin);
  char waterLevel1Str[2];
  char waterLevel2Str[2];
  itoa(waterLevel1, waterLevel1Str, 10);
  itoa(waterLevel2, waterLevel2Str, 10);
  client.publish(waterLevel1Topic, waterLevel1Str);
  client.publish(waterLevel2Topic, waterLevel2Str);

  delay(2000);  
}
