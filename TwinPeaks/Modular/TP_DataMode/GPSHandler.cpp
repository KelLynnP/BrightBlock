#include "GPSHandler.h"

GPSHandler::GPSHandler(HardwareSerial& serial) : GPS(&serial), GpsTimer(millis()) {
    readGPSData = nullptr;  // Initialize the pointer to nullptr, just to be safe.
}

GPSHandler::~GPSHandler() {
    delete readGPSData;  // Delete the allocated memory
}

void GPSHandler::setup() {
    readGPSData = new GPSData();
    GPS.begin(9600);
    GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
    GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ); // can be set to 5 hz
    GPS.sendCommand(PGCMD_NOANTENNA);
}

void GPSHandler::readAndStoreGPS() {
    char c = GPS.read();
    if (GPS.newNMEAreceived()) {
      // Serial.print(GPS.lastNMEA()); // this also sets the newNMEAreceived() flag to false
      if (GPS.parse(GPS.lastNMEA()))  // this also sets the newNMEAreceived() flag to fals
        sprintf(readGPSData->FullTimeStamp, "%02d:%02d:%02d %02d/%02d/%04d", GPS.hour, GPS.minute, GPS.seconds, GPS.day, GPS.month, GPS.year);
        sprintf(readGPSData->ShortTimeStamp, "%02d%02d%02d", GPS.hour, GPS.minute, GPS.seconds);
        readGPSData->latitude = GPS.latitudeDegrees;
        readGPSData->longitude = GPS.longitudeDegrees;
        readGPSData->altitude = GPS.altitude;
        readGPSData->dataReceived = true;
    }
}