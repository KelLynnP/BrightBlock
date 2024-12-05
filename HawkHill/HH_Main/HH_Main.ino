#include "handlers/gpsHandler.h"
#include "handlers/gpsHandler.cpp" 

#include "handlers/bleHandler.h"
#include "handlers/bleHandler.cpp" 

GPSHandler gpsHandler(Serial);
NimBLEHandler bleHandler;
const unsigned long PRINT_INTERVAL = 1000; 
unsigned long lastPrintTime = 0;
const std::string deviceName = "Bleep";

void setup() {
  // Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, GPS_RX, GPS_TX);  // For GPS
  gpsHandler.setup();
  // bleHandler.setupBLE("Bleep");

  // Add this debug line
  Serial.println("GPS Raw Data:");
}
// String rawData; // fix me later, but for now will capture all raw data

void loop() {
  while (Serial2.available()) {
    char c = Serial2.read();
    Serial.print(c);  // Simple echo of GPS data
  }
  
  gpsHandler.updateGPS();
  // bleHandler.handleConnection();
}

