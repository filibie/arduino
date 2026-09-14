#include "DHT.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

#define LED_PIN 5
#define INTERNAL_LED_PIN 2

#define REQUEST_RATE_MINUTES 5

#define DHTPIN 4
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

// Wi-Fi credentials
const char* ssid = "";
const char* password = "";
// const char* serverName = "http://192.168.1.59:8081/api/sensor";
const char* serverName = "http://fbserver:8081/api/sensor";

String deviceId;

void blinkLED(int times, int duration) {
  for (int i = 0; i < times; i ++) {
    digitalWrite(LED_PIN, HIGH);
    delay(duration);
    digitalWrite(LED_PIN, LOW);
    delay(duration);
  }
}

void blinkFast(int times) {
  for (int i = 0; i < times; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(50);
    digitalWrite(LED_PIN, LOW);
    delay(50);
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  pinMode(INTERNAL_LED_PIN, OUTPUT);
  digitalWrite(INTERNAL_LED_PIN, LOW);
  
  deviceId = "esp8266-" + String(ESP.getChipId(), HEX);
  Serial.println("Device ID: " + deviceId);
  
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected!");

  dht.begin();
  delay(500);
  digitalWrite(INTERNAL_LED_PIN, HIGH);
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    float h = dht.readHumidity();
    float t = dht.readTemperature();

    int rawLight = analogRead(A0);
    float normLight = rawLight / 1023.0; // normalized 0-1

    if (isnan(h) || isnan(t)) {
      Serial.print("Failed to read from the DHT sensor!");
      delay(2000);
      return;
    }

    Serial.print("Temp: "); Serial.print(t);
    Serial.print(" *C  Hum: "); Serial.print(h);
    Serial.print(" %  Light: "); Serial.println(normLight);

    WiFiClient client;
    HTTPClient http;
    
    http.begin(client, serverName);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Authorization", "Bearer d615cb29-136e-42c2-ae64-8450f76da7c3");
    http.writeToStream(&Serial);

    // Build JSON
    StaticJsonDocument<256> doc;
    doc["deviceId"] = deviceId;
    doc["temperature"] = t;
    doc["humidity"] = h;
    doc["light"] = normLight;

    String requestBody;
    serializeJson(doc, requestBody);

    blinkFast(10);
    int httpResponseCode = http.POST(requestBody);

    if (httpResponseCode > 0) {
      Serial.print("Response code: ");
      Serial.print(httpResponseCode);
      String payload = http.getString();
      Serial.print(", Response body: ");
      Serial.println(payload);
    } else {
      Serial.print("Server responsed with error, code: ");
      Serial.println(httpResponseCode);
    }
    
    http.end();
  }
  
  delay(60000 * REQUEST_RATE_MINUTES); // every 60s
}
