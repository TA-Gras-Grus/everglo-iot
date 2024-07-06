#include <ESP8266WiFi.h>
#include <WebSocketsClient.h>  // Include the WebSockets library
#include <BearSSLHelpers.h>    // Include BearSSL helpers

// Relay pins
#define RELAY_PIN_HEATER D3
#define RELAY_PIN_FAN D1

// Wi-Fi credentials
const char* ssid = "Tongkrongan Membiru";
const char* password = "ngisorudel";

// MQTT server
const char* mqtt_server = "02fa3ba5f83a4760bc66879c7e081d28.s1.eu.hivemq.cloud";
const int mqtt_port = 8884;  // Port for WebSocket connection
const char* mqtt_username = "everglo";
const char* mqtt_password = "everglo2024";
const char* mqtt_topic_greenhouse_update = "greenhouse:updated";
const char* mqtt_topic_greenhouse_data = "greenhouseData:created";
const char* greenhouse_api_key = "";

// Root certificate for the server
const char* root_ca = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIDrzCCApegAwIBAgIUQ0r5k8j6G1J5y5v5k8j6G1J5y5v5k8j6G1J5y5v5k8j6\n" \
"-----END CERTIFICATE-----\n";

WiFiClientSecure espClient;
WebSocketsClient webSocket;

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  // Serial.println);
  Serial.printf("[WSc url: %s\n", payload);
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.printf("[WSc] Disconnected!\n");
      break;
    case WStype_CONNECTED:
      Serial.printf("[WSc] Connected to url: %s\n", payload);
      // send message to server when Connected
      webSocket.sendTXT("Connected");
      break;
    case WStype_TEXT:
      Serial.printf("[WSc] get text: %s\n", payload);
      // handle text message from server
      break;
    case WStype_BIN:
      Serial.printf("[WSc] get binary length: %u\n", length);
      // handle binary message from server
      break;
    case WStype_ERROR:
      Serial.printf("[WSc] Error!\n");
      break;
    case WStype_PING:
      Serial.printf("[WSc] Ping!\n");
      break;
    case WStype_PONG:
      Serial.printf("[WSc] Pong!\n");
      break;
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(RELAY_PIN_HEATER, OUTPUT);
  pinMode(RELAY_PIN_FAN, OUTPUT);
  digitalWrite(RELAY_PIN_HEATER, LOW);
  digitalWrite(RELAY_PIN_FAN, LOW);

  setup_wifi();
  
  // Load root certificate
  // BearSSL::X509List cert(root_ca);
  // espClient.setTrustAnchors(&cert);
  
  webSocket.setAuthorization(mqtt_username, mqtt_password); // user, password
  webSocket.begin(mqtt_server, mqtt_port, "/mqtt"); // server address, port and URL
  webSocket.onEvent(webSocketEvent);
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

void loop() {
  webSocket.loop();
  delay(2000);
}
