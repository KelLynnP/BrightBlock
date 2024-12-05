// GPS Handler MainTest
#include "../../handlers/gpsHandler.h"
#include "../../handlers/gpsHandler.cpp" 

// Define GPS Serial pins for ESP32
#define GPS_RX 16  // U2RXD
#define GPS_TX 17  // U2TXD

// Use Serial2 instead of Serial1 for ESP32 UART2
GPSHandler gpsHandler(Serial2);

const unsigned long PRINT_INTERVAL = 1000; 
unsigned long lastPrintTime = 0;

void setup() {
  Serial.begin(115200);  // Start Serial for debugging output
  Serial2.begin(9600, SERIAL_8N1, GPS_RX, GPS_TX);  // Initialize UART2 for GPS
  delay(1000);          // Give time for serial ports to initialize
  
  Serial.println("GPS Test Starting...");
  gpsHandler.setup();
  Serial.println("GPS Handler Initialized");
}

void loop() {
  gpsHandler.updateGPS();

  unsigned long currentTime = millis();
  if (currentTime - lastPrintTime >= PRINT_INTERVAL) {
    // Print raw characters from GPS
    while (Serial2.available()) {
      char c = Serial2.read();
      Serial.print(c);
    }    
    
    // Add some debug information
    Serial.print("Fix: "); Serial.println(gpsHandler.hasValidFix());
    Serial.print("Time: "); Serial.println(gpsHandler.getTimeStamp());
    if (gpsHandler.hasValidFix()) {
      Serial.print("Lat: "); Serial.println(gpsHandler.getLatitude(), 6);
      Serial.print("Lon: "); Serial.println(gpsHandler.getLongitude(), 6);
      Serial.print("Alt: "); Serial.println(gpsHandler.getAltitude());
    }
    Serial.println("--------------------");
    lastPrintTime = currentTime;
  }
}

