#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define i2c_Address 0x3C

#define OLED_RESET -1

Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


void setup() {
  Wire.begin(D2, D1);

  if(!display.begin(i2c_Address, true)) {
    for(;;);
  }

  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0, 10);

  // wypisanie tekstu
  display.println("Adafruit SH110X");
  display.setCursor(0, 30);
  display.println("Działa na NodeMCU!");

  // Rysowanie prostej linii (funkcja z biblioteki GFX)
  display.drawLine(0, 50, 128, 50, SH110X_WHITE);

  // Wysłanie bufora do pamięci ekranu (bez tego nic się nie pojawi)
  display.display();
}

void loop() {
  // put your main code here, to run repeatedly:

}
