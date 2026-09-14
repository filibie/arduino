#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <U8g2lib.h>
#include <Wire.h>

const char* ssid        = "";
const char* password    = "";

const char* apiURL      = "https://api.open-meteo.com/v1/forecast?latitude=49.58&longitude=20.25&current=temperature_2m,wind_speed_10m&hourly=temperature_2m,relative_humidity_2m,wind_speed_10m";

// --- Non-blocking Intervals ---
const unsigned long REFRESH_INTERVAL   = 60000; // 60 seconds
const unsigned long RENDER_INTERVAL    = 100;   // 100ms UI update

unsigned long lastApiTime   = 0;
unsigned long lastRenderTime= 0;

// --- State Variables ---
String btcPriceDisplay = "Fetching...";
String weatherDisplay = "Fetching...";

// Explicit hardware I2C pins for NodeMCU: SDA = D2 (GPIO4), SCL = D1 (GPIO5)
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ D1, /* data=*/ D2);

void showStatus(const char* line1, const char* line2 = "") {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.drawStr(5, 20, line1);
  u8g2.drawStr(5, 40, line2);
  u8g2.sendBuffer();
}

void fetchWeather() {
  digitalWrite(LED_BUILTIN, LOW); // LED ON

  if (WiFi.status() != WL_CONNECTED) {
    btcPriceDisplay = "WiFi Offline";
    WiFi.reconnect();
    digitalWrite(LED_BUILTIN, HIGH);
    return;
  }

  // Allocate client on heap/stack with minimal SSL buffer size to prevent RAM starvation
  WiFiClientSecure client;
  client.setInsecure();
  client.setBufferSizes(2048, 512); 

  HTTPClient http;

  Serial.println("Fetching weather data...");

  if (http.begin(client, apiURL)) {
    // http.setUserAgent("ESP8266-Bitcoin-Tracker");
    http.setTimeout(5000); // 5 sec timeout to avoid blocking execution

    int httpCode = http.GET();
    Serial.println(httpCode);
    if (httpCode > 0) {
      String payload = http.getString();
      Serial.println("Payload: " + payload);
    
      // JsonDocument dynamically sized for modern ArduinoJson
      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, payload);

      if (!error) {
        
        float temperature = doc["current"]["temperature_2m"];
        float wind = doc["current"]["wind_speed_10m"];

        weatherDisplay = String(temperature, 1) + String(doc["hourly_units"]["temperature_2m"]) + "  " + String(wind, 0) + String(doc["hourly_units"]["wind_speed_10m"]);
        Serial.println(weatherDisplay);

      } else {
        Serial.print("JSON Parse Error: ");
        Serial.println(error.c_str());
        weatherDisplay = "JSON Error";
      }
    } else {
      Serial.printf("HTTP Error: %s\n", http.errorToString(httpCode).c_str());
      weatherDisplay = "HTTP Error";
    }

    http.end();
  } else {
    Serial.println("Unable to connect to HTTPS endpoint");
    weatherDisplay = "Conn Failed";
  }

  digitalWrite(LED_BUILTIN, HIGH); // LED OFF
}

void renderDisplay() {
  u8g2.clearBuffer();

  // 1. Header Title
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.setCursor(5, 10);
  u8g2.print("Pogoda Bochnia");

  // 2. Price Display
  u8g2.setFont(u8g2_font_ncenR10_tf);
  u8g2.setCursor(5, 38);
  u8g2.print(weatherDisplay.c_str());

  // 3. Full-Width Progress Bar
  unsigned long elapsed = millis() - lastApiTime;
  if (elapsed > REFRESH_INTERVAL) elapsed = REFRESH_INTERVAL;

  int barWidth  = 118;
  int barHeight = 7;
  int x = 5;
  int y = 53;

  int fillWidth = map(elapsed, 0, REFRESH_INTERVAL, 0, barWidth);

  u8g2.drawFrame(x, y, barWidth, barHeight);
  if (fillWidth > 2) {
    u8g2.drawBox(x + 1, y + 1, fillWidth - 2, barHeight - 2);
  }

  u8g2.sendBuffer();
}

void setup() {
  Serial.begin(115200);
  
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH); // Off

  // Initialize Wire explicitly with NodeMCU pins
  Wire.begin(D2, D1);
  Wire.setClock(400000); // Set 400kHz Fast I2C speed
  u8g2.begin();
  u8g2.enableUTF8Print();
  
  showStatus("Connecting WiFi...", ssid);

  // WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  digitalWrite(LED_BUILTIN, LOW); // On while connecting

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi Connected!");
    
    for (int i = 0; i < 2; i++) {
      digitalWrite(LED_BUILTIN, LOW);  
      delay(100);
      digitalWrite(LED_BUILTIN, HIGH); 
      delay(100);
    }

    showStatus("WiFi Connected!", WiFi.localIP().toString().c_str());
    delay(1000);
    
    lastApiTime = millis();
    fetchWeather();
  } else {
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.println("\nWiFi Failed!");
    showStatus("WiFi Connection", "Failed!");
  }
}

void loop() {
  unsigned long currentMillis = millis();

  // Task 1: Render Display (Prioritized for UI smoothness)
  if (currentMillis - lastRenderTime >= RENDER_INTERVAL) {
    lastRenderTime = currentMillis;
    renderDisplay();
  }

  // Task 2: API Refresh
  if (currentMillis - lastApiTime >= REFRESH_INTERVAL) {
    lastApiTime = currentMillis;
    fetchWeather();
  }

  yield(); // Keep ESP8266 background tasks & Watchdog Timer happy
}
