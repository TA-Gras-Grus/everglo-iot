#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

const char* ssid = "WARMINDO RENEO";
const char* password = "reneowae";

// Tambahkan variabel untuk EC dan PPM
float ecValue;
float ppmValue;

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
  // Tambahkan pembacaan EC dan PPM dari sensor EC
  int ecValueRaw = analogRead(A0); // Baca nilai raw dari pin A0

  // Konversi nilai raw ke tegangan (0-5V)
  float voltage = ecValueRaw * (5.0 / 1023.0);

  // Konversi tegangan ke µS/cm (0-4400 µS/cm)
  ecValue = voltage * (4400.0 / 5.0);

  // Konversi µS/cm ke PPM dengan faktor konversi tipikal 0.5
  ppmValue = ecValue * 0.5;

  // Cek dan kirim data dari sensor EC jika terbaca dan tidak bernilai 0
  if (!isnan(ecValue) && !isnan(ppmValue) && !(ecValue == 0 && ppmValue == 0)) {
    patchEcAndPpm(ecValue, ppmValue);
  } else {
    Serial.println("EC atau PPM tidak terbaca atau bernilai 0... !");
  }

  delay(10000); // Kirim permintaan setiap 10 detik
}

void patchEcAndPpm(float ec, float ppm) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    String url = "https://everglo-backend.vercel.app/api/greenhouses/update-greenhouse-iot?apiKey=2e3ae7b6-eee8-4b48-8871-c0c7c4659b15";
    http.begin(espClient, url); // Tentukan URL
    http.addHeader("Content-Type", "application/json"); // Tentukan header tipe konten

    // JSON request body
    String requestBody = "{\"ec\":" + String(ec) + ",\"ppm\":" + String(ppm) + "}";
    Serial.println(requestBody);

    int httpResponseCode = http.PATCH(requestBody); // Kirim permintaan

    if (httpResponseCode > 0) {
      String response = http.getString(); // Dapatkan respons terhadap permintaan
      Serial.println(httpResponseCode); // Cetak kode balasan
      Serial.println(response); // Cetak jawaban permintaan
    } else {
      Serial.print("Error on sending PATCH: ");
      Serial.println(httpResponseCode);
    }

    http.end(); // Bebaskan sumber daya
  }
}
