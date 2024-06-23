#ifndef GPSHANDLER_H
#define GPSHANDLER_H

#include <Adafruit_GPS.h>
#include <Arduino.h>

class GPSHandler {
private:
    uint32_t GpsTimer;
    const int sampleRateGps = 5000;
    struct GPSData {
      char FullTimeStamp[20];
      char ShortTimeStamp[6];
      float latitude;
      float longitude;
      float altitude;
      bool dataReceived = false;
        };
    GPSData* readGPSData;

public:
    Adafruit_GPS GPS;
    GPSHandler(HardwareSerial& serial);
    ~GPSHandler(); // Destructor :,)

    void setup();
    void readAndStoreGPS();

    // public access functions 
    const char* getFullTimeStamp() const {return readGPSData->FullTimeStamp;}
    const char* getShortTimeStamp() const {return readGPSData->ShortTimeStamp;}
    float getLatitude() const {return readGPSData->latitude; }
    float getLongitude() const { return readGPSData->longitude; }
    float getAltitude() const { return readGPSData->altitude; }

};

#endif // GPSHANDLER_H