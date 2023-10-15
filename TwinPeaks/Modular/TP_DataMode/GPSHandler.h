#ifndef GPSHANDLER_H
#define GPSHANDLER_H

#include <Adafruit_GPS.h>
#include <Arduino.h>#ifndef GPSHANDLER_H
#define GPSHANDLER_H

#include <Adafruit_GPS.h>
#include <Arduino.h>

class GPSHandler {
private:
    Adafruit_GPS GPS;
    uint32_t GpsTimer;
    const int sampleRateGps = 5000;

public:
    struct GPSData {
        char FullTimeStamp[20];
        char ShortTimeStamp[6];
        float latitude;
        float longitude;
        float altitude;
        bool dataReceived = false;
    };

    GPSHandler(HardwareSerial& serial);
    void setup();
    GPSData readAndStoreGPS();
};

#endif // GPSHANDLER_H