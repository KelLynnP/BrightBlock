#include "gpsHandler.h"

GPSHandler::GPSHandler(HardwareSerial& serial) : GPS(&serial) {
    readGPSData = nullptr; 
}

GPSHandler::~GPSHandler() {
    delete readGPSData;  
}

void GPSHandler::setup() {
    readGPSData = new GPSData();
    GPS.begin(9600); // From previous GPS (needs to be tested!)
    GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
    GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ); 
    GPS.sendCommand(PGCMD_NOANTENNA);
}

void GPSHandler::updateGPS() {
    if (!readGPSData) return;  
    
    char c = GPS.read();
    if (GPS.newNMEAreceived()) {
        if (GPS.parse(GPS.lastNMEA())) {
            sprintf(readGPSData->TimeStamp, 
                    "%02d:%02d:%02d %02d/%02d/%04d",
                    GPS.hour, GPS.minute, GPS.seconds, 
                    GPS.day, GPS.month, GPS.year);
            
            readGPSData->hasFix = GPS.fix; // Fix != TimeStamp Values
            
            if (GPS.fix) {
                readGPSData->latitude = GPS.latitudeDegrees;
                readGPSData->longitude = GPS.longitudeDegrees;
                readGPSData->altitude = GPS.altitude;
            }
            readGPSData->lastUpdate = millis();
        }
    }
}