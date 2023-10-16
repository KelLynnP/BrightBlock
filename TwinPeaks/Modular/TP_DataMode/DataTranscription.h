#ifndef DATATRANSCIPTION_H
#define DATATRANSCIPTION_H

#include <string>
#include <vector>

class GPSHandler;  
class Sen55Handler;    
class Button;

class DataTranscription {

private:

public:
    std::vector<std::string> PullAndTranscribeData(const GPSData& GPSData2Transmit, const Sen55& Sen55Data);
    std::string FormatAndAppendTimestamp(float RawData, const char* TimeSnip);
};

#endif // DATATRANSCIPTION_H

