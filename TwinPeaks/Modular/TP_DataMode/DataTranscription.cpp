#include "DataTranscription.h"
#include "GPSHandler.h"
#include "Sen55Handler.h"
#include "Button.h"

std::vector<std::string> DataTranscription::PullAndTranscribeData(const GPSData& GPSData2Transmit, const Sen55& Sen55Data, int buttonCounts ) {
  std::vector<std::string> sensorDataVector(NumCharacteristics);
  // GPS Data All pulled seperatly
  sensorDataVector[0] = GPSData2Transmit.FullTimeStamp;

  sensorDataVector[1] = FormatAndAppendTimestamp(GPSData2Transmit.latitude, GPSData2Transmit.ShortTimeStamp);
  sensorDataVector[2] = FormatAndAppendTimestamp(GPSData2Transmit.longitude, GPSData2Transmit.ShortTimeStamp);
  sensorDataVector[3] = FormatAndAppendTimestamp(GPSData2Transmit.altitude, GPSData2Transmit.ShortTimeStamp);
  sensorDataVector[4] = FormatAndAppendTimestamp(Sen55Data.massConcentrationPm2p5, GPSData2Transmit.ShortTimeStamp);

  //relative Humidity
  sensorDataVector[5] = FormatAndAppendTimestamp(Sen55Data.ambientHumidity, GPSData2Transmit.ShortTimeStamp);
  sensorDataVector[6] = FormatAndAppendTimestamp(Sen55Data.ambientTemperature, GPSData2Transmit.ShortTimeStamp);

  // Filling in with more climate data for now
  sensorDataVector[7] = FormatAndAppendTimestamp(Sen55Data.vocIndex, GPSData2Transmit.ShortTimeStamp);
  sensorDataVector[8] = FormatAndAppendTimestamp(Sen55Data.noxIndex, GPSData2Transmit.ShortTimeStamp);
  sensorDataVector[9] = FormatAndAppendTimestamp(Sen55Data.massConcentrationPm10p0, GPSData2Transmit.ShortTimeStamp);
 
 // sensorDataVector[10] = FormatAndAppendTimestamp(buttonCounts, GPSData2Transmit.ShortTimeStamp);

  return sensorDataVector;
}
// maybe types here will cause errors!
std::string DataTranscription::FormatAndAppendTimestamp(float RawData, const char* TimeSnip) {
  char Data[15];
  // Serial.println(RawData);
  sprintf(Data, ",%f", RawData);
  std::string str = std::string(TimeSnip) + Data;
  Serial.println(str.c_str());
  return str;
}