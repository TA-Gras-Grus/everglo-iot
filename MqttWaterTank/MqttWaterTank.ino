#include <ArduinoMqttClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include "arduino_secrets.h"

char ssid[] = SECRET_SSID;  
char pass[] = SECRET_PASS;  

WiFiClientSecure wifiClient;
MqttClient mqttClient(wifiClient);

const char broker[] = "a7701bd2b3e54353b8aeab74b9c7f322.s1.eu.hivemq.cloud";
int port = 8883;
const char topic[] = "greenhouse:updated";  // Subscribe to all topics
const char* greenhouse_key = "2d6221b2-6c0a-4fa8-876a-55d2f5ce6de7";
const int floatSensor1 = D4;
const int floatSensor2 = D3;
int buttonState1 = 1;  //reads pushbutton status
int buttonState2 = 1;  //reads pushbutton status
const int relayDosing1 = D2;
const int relayDosing2 = D1;
const int relayPump = D0;
float ecValue, ppmValue;  //deklarasi variabel
const long interval = 1000;
unsigned long previousMillis = 0;

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, pass);

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
  pinMode(floatSensor1, INPUT_PULLUP);
  pinMode(floatSensor2, INPUT_PULLUP);
  pinMode(relayPump, OUTPUT);
  pinMode(relayDosing1, OUTPUT);
  pinMode(relayDosing2, OUTPUT);

  digitalWrite(relayPump, LOW);
  digitalWrite(relayDosing1, LOW);
  digitalWrite(relayDosing2, LOW);

  while (!Serial) {
    ;  // wait for serial port to connect. Needed for native USB port only
  }
  setup_wifi();

  mqttClient.setId("waterTank");

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
  mqttClient.poll();
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    // Tambahkan pembacaan EC dan PPM dari sensor EC
    int ecValueRaw = analogRead(A0);  // Baca nilai raw dari pin A0

    // Konversi nilai raw ke tegangan (0-5V)
    float voltage = ecValueRaw * (5.0 / 1023.0);

    // Konversi tegangan ke µS/cm (0-4400 µS/cm)
    ecValue = voltage * (4400.0 / 5.0);

    // Konversi µS/cm ke PPM dengan faktor konversi tipikal 0.5
    ppmValue = ecValue * 0.5;

    buttonState1 = digitalRead(floatSensor1);
    buttonState2 = digitalRead(floatSensor2);

    // Cek dan kirim data dari sensor EC jika terbaca dan tidak bernilai 0
    if (!isnan(ecValue) && !isnan(ppmValue) && !(ecValue == 0 && ppmValue == 0)) {
      if (ppmValue < 150) {
        startDosing();
      } else {
        stopDosing();
      }
      updateValue(ecValue, ppmValue);
    } else {
      Serial.println("EC atau PPM tidak terbaca atau bernilai 0... !");
    }
    if (buttonState1 == HIGH && buttonState2 == HIGH) {
      stopPump();
    }
    if (buttonState1 == LOW && buttonState2 == LOW) {
      startPump();
    }
  }
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
}

void updateValue(float ecValue, float ppmValue) {
  bool retained = false;
  int qos = 1;
  bool dup = false;
  String requestBody = "{\"deviceId\":\"" + String(greenhouse_key) + "\",\"ec\":" + String(ecValue) + ",\"ppm\":" + String(ppmValue) + "}";
  Serial.println("Greenhouse updated : " + requestBody);
  mqttClient.beginMessage("greenhouse-iot:updated", requestBody.length(), retained, qos, dup);
  mqttClient.print(requestBody);
  mqttClient.endMessage();
}

void startPump() {
  digitalWrite(relayPump, LOW);
  bool retained = false;
  int qos = 1;
  bool dup = false;
  String requestBody = "{\"deviceId\":\"" + String(greenhouse_key) + "\",\"statusWaterTank\":false}";
  Serial.println("Water level updated : " + requestBody);
  mqttClient.beginMessage("greenhouse-iot:updated", requestBody.length(), retained, qos, dup);
  mqttClient.print(requestBody);
  mqttClient.endMessage();
}

void stopPump() {
  digitalWrite(relayPump, HIGH);
  bool retained = false;
  int qos = 1;
  bool dup = false;
  String requestBody = "{\"deviceId\":\"" + String(greenhouse_key) + "\",\"statusWaterTank\":true}";
  Serial.println("Water level updated : " + requestBody);
  mqttClient.beginMessage("greenhouse-iot:updated", requestBody.length(), retained, qos, dup);
  mqttClient.print(requestBody);
  mqttClient.endMessage();
}

void startDosing() {
  digitalWrite(relayDosing1, LOW);
  digitalWrite(relayDosing2, LOW);
}

void stopDosing() {
  digitalWrite(relayDosing1, HIGH);
  digitalWrite(relayDosing2, HIGH);
}
