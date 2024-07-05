#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <DHT.h> //library DHT

#define DHTPIN D1 //pin DATA konek ke pin 2 Arduino
#define DHTTYPE DHT11 //tipe sensor DHT11
DHT dht(DHTPIN, DHTTYPE); //set sensor + koneksi pin
float humi, temp;//deklarasi variabel 
const char* ssid = "Jembut";
const char* password = "katsugosong";

WiFiClientSecure espClient;

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  espClient.setInsecure(); 

  Serial.println("Connected to WiFi");
}

void loop() {
  humi = dht.readHumidity();//baca kelembaban
  temp = dht.readTemperature();//baca suhu
  if (isnan(humi) || isnan(temp)) { //jika tidak ada hasil
    Serial.println("DHT11 tidak terbaca... !");
    delay(2000);  
    return;
  }
  else{//jika ada hasilnya 
    updateGreenhouse(temp, humi);
    delay(10000); // Send a request every 10 seconds
  }
}

void updateGreenhouse(float temperature, float humidity){
    if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    http.begin(espClient, "https://everglo-backend.vercel.app/api/greenhouses/create-greenhouse-data?apiKey=2e3ae7b6-eee8-4b48-8871-c0c7c4659b15"); // Specify the URL
    http.addHeader("Content-Type", "application/json"); // Specify content-type header

    // JSON request body
    String requestBody = "{\"airTemperature\":"+String(temperature)+",\"humidity\":"+String(humidity)+"}";
    Serial.println(requestBody);

    int httpResponseCode = http.POST(requestBody); // Send the request

    if (httpResponseCode > 0) {
      String response = http.getString(); // Get the response to the request
      Serial.println(httpResponseCode); // Print return code
      Serial.println(response); // Print request answer
    } else {
      Serial.print("Error on sending POST: ");
      Serial.println(httpResponseCode);
    }

    http.end(); // Free resources
  }
}
