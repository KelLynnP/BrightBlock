#include <Adafruit_GPS.h>
#include <Arduino.h>

class GPSHandler {
private:
    Adafruit_GPS GPS;
    uint32_t GpsTimer;
    const int sampleRateGps = 5000;

public:
    GPSHandler(HardwareSerial& serial) : GPS(&serial), GpsTimer(millis()) {}
    
    void setup() {
        GPS.begin(9600);
        GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
        GPS.sendCommand(PMTK_SET_NMEA_UPDATE_5HZ);
    }

    struct GPSData {
      char FullTimeStamp[20];
      char ShortTimeStamp[6];
      float latitude;
      float longitude;
      float altitude;
      bool dataReceived = false;
    };

    GPSData readAndStoreGPS() {
        GPSData TempGPS;
        char c = GPS.read();
        // Serial.print(c);

        if (GPS.newNMEAreceived()) {
            if (!GPS.parse(GPS.lastNMEA())) return TempGPS;
        }

        if ((millis() - GpsTimer > sampleRateGps) && (!GPS.parse(GPS.lastNMEA())) && (GPS.fix != 0)) {
            GpsTimer = millis();
            sprintf(TempGPS.FullTimeStamp, "%02d:%02d:%02d %02d/%02d/%04d", GPS.hour, GPS.minute, GPS.seconds, GPS.day, GPS.month, GPS.year);
            sprintf(TempGPS.ShortTimeStamp, "%02d%02d%02d", GPS.hour, GPS.minute, GPS.seconds);
            TempGPS.latitude = GPS.latitudeDegrees;
            TempGPS.longitude = GPS.longitudeDegrees;
            TempGPS.altitude = GPS.altitude;
            TempGPS.dataReceived = true;
            return TempGPS;
        } else {
            sprintf(TempGPS.FullTimeStamp, "%02d:%02d:%02d %02d/%02d/%04d", 90, 90, 90, 90, 90, 1999);
            sprintf(TempGPS.ShortTimeStamp, "%02d%02d%02d", 90, 90, 90);
            TempGPS.latitude = -1;
            TempGPS.longitude = -1;
            TempGPS.altitude = -1;
            TempGPS.dataReceived = false;
            return TempGPS;
        }
    }
};

// Sample usage:
GPSHandler gpsHandler(Serial1);

void setup() {
    Serial.begin(115200);
    gpsHandler.setup();
}

void loop() {
    GPSHandler::GPSData gpsDataInstance = gpsHandler.readAndStoreGPS();
    if (gpsDataInstance.dataReceived) {
        // Process the data
        Serial.print(gpsDataInstance.FullTimeStamp);
    }else{
      // Serial.print("no fix");
    }

}

// Functional/ not object orieneted 

// #include <Adafruit_GPS.h>
// #include <SoftwareSerial.h>
// #include <vector>
// #include <string>

// // ---------------------------------------------------------------//
// // ------------------ GPS information ----------------------------//
// // ---------------------------------------------------------------//
// #define GPSSerial Serial1
// Adafruit_GPS GPS(&GPSSerial);
// #define GPSECHO false
// uint32_t GpsTimer = millis();
// uint32_t GlobalTimer = millis();
// const int sampleRateGps = 5000;  // update line in initiation if this changes

// struct GPSData {
//   char FullTimeStamp[20];
//   char ShortTimeStamp[6];
//   float latitude;
//   float longitude;
//   float altitude;
//   bool dataReceived = false;  // set true if set correctly
// };

// GPSData readAndStoreGPS() {
//   GPSData TempGPS;
//   char c = GPS.read();
//   // Serial.print(c);
//   if (GPSECHO) {
//     if (c) {
//       Serial.print(c);  
//     }
//   }

//   if (GPS.newNMEAreceived()) {
//     if (!GPS.parse(GPS.lastNMEA())) 
//       return TempGPS;
//   }

//   if ((millis() - GpsTimer > sampleRateGps) && (!GPS.parse(GPS.lastNMEA())) && (GPS.fix != 0)) {
//     GpsTimer = millis();  // reset the timer
//     sprintf(TempGPS.FullTimeStamp, "%02d:%02d:%02d %02d/%02d/%04d", GPS.hour, GPS.minute, GPS.seconds, GPS.day, GPS.month, GPS.year);
//     sprintf(TempGPS.ShortTimeStamp, "%02d%02d%02d", GPS.hour, GPS.minute, GPS.seconds);
//     TempGPS.latitude = GPS.latitudeDegrees;
//     TempGPS.longitude = GPS.longitudeDegrees;
//     TempGPS.altitude = GPS.altitude;
//     TempGPS.dataReceived = true;
//     return TempGPS;
//   } else {
//     GPSData TempGPS;
//     sprintf(TempGPS.FullTimeStamp, "%02d:%02d:%02d %02d/%02d/%04d", 90, 90, 90, 90, 90, 1999);
//     sprintf(TempGPS.ShortTimeStamp, "%02d%02d%02d", 90, 90, 90);
//     TempGPS.latitude = -1;
//     TempGPS.longitude = -1;
//     TempGPS.altitude = -1;
//     TempGPS.dataReceived = false;
//     return TempGPS;
//   }
// }

// void setup() {
//   Serial.begin(115200);

//   //----------- GPS Set up-------------- //
//   GPSSerial.begin(9600);  
//   GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
//   GPS.sendCommand(PMTK_SET_NMEA_UPDATE_5HZ);  // 1 Hz update rate
// }

// void loop() {
//   // You can call readAndStoreGPS() function here to get the GPS data in loop
//   GPSData gpsData = readAndStoreGPS();
//   // process gpsData if needed...
// }
