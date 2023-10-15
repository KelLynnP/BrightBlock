#include "GPSHandler.h"

GPSHandler::GPSHandler(HardwareSerial& serial) : GPS(&serial), GpsTimer(millis()) {}

void GPSHandler::setup() {
    GPS.begin(9600);
    GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
    GPS.sendCommand(PMTK_SET_NMEA_UPDATE_5HZ);
}

GPSHandler::GPSData GPSHandler::readAndStoreGPS() {
    GPSData TempGPS;
    char c = GPS.read();

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
