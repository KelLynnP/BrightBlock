// GPS Handler MainTest
#include "../../handlers/gpsHandler.h"
#include "../../handlers/gpsHandler.cpp" 

GPSHandler gpsHandler(Serial1);

const unsigned long PRINT_INTERVAL = 1000; 
unsigned long lastPrintTime = 0;

void setup() {
  Serial.begin(115200);  // Start Serial for debugging output
  delay(1000);          // Give time for serial ports to initialize
  
  Serial.println("GPS Test Starting...");
  gpsHandler.setup();
  Serial.println("GPS Handler Initialized");
}

void loop() {
  // Update GPS data
  gpsHandler.updateGPS();

  unsigned long currentTime = millis();
  if (currentTime - lastPrintTime >= PRINT_INTERVAL) {
  // Add raw character output

  while (Serial1.available()) {
    char c = Serial1.read();
    Serial.print(c);  // Print raw characters from GPS
  }    
  lastPrintTime = currentTime;
  }
}

