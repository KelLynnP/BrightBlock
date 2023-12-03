#include "DataTranscription.h"

std::string DataTranscription::FormatAndAppendTimestamp(float RawData, const char* TimeSnip) {
  char Data[15];
  sprintf(Data, ",%f", RawData);
  std::string str = std::string(TimeSnip) + Data;
  return str;
}

