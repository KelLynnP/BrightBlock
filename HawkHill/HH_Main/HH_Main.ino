#include "handlers/gpsHandler.h"
#include "handlers/gpsHandler.cpp" 

#include "handlers/bleHandler.h"
#include "handlers/bleHandler.cpp" 

GPSHandler gpsHandler(Serial);
NimBLEHandler bleHandler;
const unsigned long PRINT_INTERVAL = 1000; 
unsigned long lastPrintTime = 0;

void setup() {
  delay(1000);
  
  gpsHandler.setup();
  bleHandler.setupBLE("Bleep");
}

void loop() {
  gpsHandler.updateGPS();
  bleHandler.handleConnection();

  unsigned long currentTime = millis();
  if (currentTime - lastPrintTime >= PRINT_INTERVAL) {
    if (bleHandler.isConnected()) {
      // Update GPS-related characteristics
      bleHandler.updateCharacteristic("TimeStamp", gpsHandler.getTimeStamp());
      bleHandler.updateCharacteristic("GPSFix", String(gpsHandler.hasValidFix()).c_str());
      
      if (gpsHandler.hasValidFix()) {
        bleHandler.updateCharacteristic("Latitude", String(gpsHandler.getLatitude(), 6).c_str());
        bleHandler.updateCharacteristic("Longitude", String(gpsHandler.getLongitude(), 6).c_str());
        bleHandler.updateCharacteristic("Altitude", String(gpsHandler.getAltitude(), 2).c_str());
      }
      
      // Send raw NMEA data through BulkData characteristic
      String rawData;
      while (Serial.available()) {
        char c = Serial.read();
        rawData += c;
      }
      if (rawData.length() > 0) {
        bleHandler.updateCharacteristic("BulkData", rawData.c_str());
      }
    }
    lastPrintTime = currentTime;
  }
}