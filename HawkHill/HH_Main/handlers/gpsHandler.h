#ifndef GPS_HANDLER_H
#define GPS_HANDLER_H

#include <Adafruit_GPS.h>
#include <Arduino.h>

class GPSHandler {
private:
    const int sampleRateGps = 5000;
    struct GPSData {
      char TimeStamp[20];
      float latitude;
      float longitude;
      float altitude;
      bool hasFix;
      unsigned long lastUpdate;
    };
    GPSData* readGPSData;

public:
    Adafruit_GPS GPS;
    GPSHandler(HardwareSerial& serial);
    ~GPSHandler(); // Destructor 

    void setup();
    void updateGPS();

    bool hasValidFix() const {
        if (!readGPSData) return false;
        return readGPSData->hasFix;
    }
    // Getters
    const char* getTimeStamp() const { 
        return readGPSData ? readGPSData->TimeStamp : ""; 
    }

    float getLatitude() const { 
        return readGPSData ? readGPSData->latitude : 0.0f; 
    }

    float getLongitude() const { 
        return readGPSData ? readGPSData->longitude : 0.0f; 
    }

    float getAltitude() const { 
        return readGPSData ? readGPSData->altitude : 0.0f; 
    }
};

#endif // GPSHANDLER_H
