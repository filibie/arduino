#include <Wire.h>

void setup() {
  Wire.begin(D2, D1); // SDA=D2, SCL=D1
  Serial.begin(115200);
  while (!Serial); 
  Serial.println("\n--- Skanowanie Magistrali I2C ---");
}

void loop() {
  byte error, address;
  int nDevices = 0;

  for (address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("Znaleziono urzadzenie pod adresem: 0x");
      if (address < 16) Serial.print("0");
      Serial.print(address, HEX);
      Serial.println(" !");
      nDevices++;
    }
  }
  if (nDevices == 0) {
    Serial.println("Nie znaleziono ZADNYCH urzadzen I2C. Sprawdz polaczenia.");
  }
  delay(3000);
}
