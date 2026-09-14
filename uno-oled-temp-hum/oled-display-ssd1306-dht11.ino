#include <DHT.h>
#include <DHT_U.h>

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define LEFT_MARGIN 10

// screen timer
#define SCREEN_CHANGE_INTERVAL 5000
long previousScreenTimerMillis = 0;
int screenId = 1;

#define DHTPIN 2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// sensor read timer
#define SENSOR_TIMER_INTERVAL 2000
long previousSensorTimerMillis = 0;

// sensor data max values
int maxTempValue = 0;
int tempValue = 0;
int maxHumidityValue = 0;
int humidityValue = 0;

void setup() 
{
  Serial.begin(9600);
  dht.begin();
  readSensorData();
  
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  displaySetup();
}

void loop() 
{
  long currentMillis = millis();
  
  // get sensor data
  if (currentMillis - previousSensorTimerMillis > SENSOR_TIMER_INTERVAL)
  {
    readSensorData();    
    previousSensorTimerMillis = currentMillis;  
  }
  
  // change screen
  if (currentMillis - previousScreenTimerMillis > SCREEN_CHANGE_INTERVAL) 
  {
    Serial.println(currentMillis - previousScreenTimerMillis);
    previousScreenTimerMillis = currentMillis;
    screenId = (screenId + 1) % 3;
  }

  switch (screenId) 
  { 
    case 1:
      // temperatura
      displayTemperatureScreen();
    break;
    case 2:
      // wilgotnosc
      displayHumidityScreen();
    break;
    default:
      displayDefaultScreen();
    break;
  }
}

void readSensorData() 
{
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  if (isnan(t) || isnan(h)) 
  {
    Serial.println("Blad odczytu danych z czujnika.");
  }
  else 
  {
    tempValue = (int)t;
    humidityValue = (int)h;
    Serial.print(t);
    Serial.print(" ");
    Serial.println(h);
    if (tempValue > maxTempValue) 
    {
      maxTempValue = tempValue;
    }
    if (humidityValue > maxHumidityValue) 
    {
      maxHumidityValue = humidityValue;
    }
  }
}

void displaySetup(void) 
{
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.cp437(true);
  display.clearDisplay();
  display.display();
  delay(1000);
}

void displayTemperatureScreen(void) 
{ 
  display.clearDisplay();

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.write("Temperatura:");
  display.setCursor(0, 10);
  display.setTextSize(2);
  display.setCursor(LEFT_MARGIN, 10);
  display.print(tempValue);
  display.write((char)248);
  display.write("C");
  
  display.setTextSize(1);
  display.setCursor(0, 35);
  display.write("Temperatura maksym.:");
  display.setCursor(0, 35);
  display.setTextSize(2);
  display.setCursor(LEFT_MARGIN, 45);
  display.print(maxTempValue);
  display.write((char)248);
  display.write("C");
  
  display.display();
}

void displayHumidityScreen(void) 
{
  display.clearDisplay();
  
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.write("Wilgotnosc:");
  display.setCursor(0, 10);
  display.setTextSize(2);
  display.setCursor(LEFT_MARGIN, 10);
  display.print(humidityValue);
  display.write("%");
  
  display.setTextSize(1);
  display.setCursor(0, 35);
  display.write("Wilgotnosc maksym.:");
  display.setCursor(0, 35);
  display.setTextSize(2);
  display.setCursor(LEFT_MARGIN, 45);
  display.print(maxHumidityValue);
  display.write("%");
  
  display.display();
}

void displayDefaultScreen(void) 
{
  display.clearDisplay();

  display.setTextSize(1);
  display.setCursor(10, 10);
  display.write("Temperatura: ");
  display.print(tempValue);
  display.write((char)248);
  display.write("C");
  
  display.setCursor(10, 25);
  display.write("Temp.max: ");
  display.print(maxTempValue);
  display.write((char)248);
  display.write("C");
  
  display.setCursor(10, 40);
  display.write("Wilgotnosc: ");
  display.print(humidityValue);
  display.print("%");
  
  display.setCursor(10, 55);
  display.write("Wilg.max: ");
  display.print(maxHumidityValue);
  display.print("%");

  display.display();
}
