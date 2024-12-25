#ifndef MEMORYCARDHANDLER_H
#define MEMORYCARDHANDLER_H

#include <Arduino.h>
#include <SPI.h>
#include <SD.h>

class MemoryCardHandler {

private:
    int eventCounter = -1;
    const char eventIndexPath[30] = "/eventIndex.json"; 
    char NewFilePath[30];  // Increased the buffer size
    void createEventIndexFile();
    uint64_t cardSize;
    uint8_t cardType;

    
public:
    bool printOut = false;
    int memoryCardPin = 4; // chip select pin
    int PullLastEventIndex();
    void setNewDataEvent();
    void writeFile(const char* path, const char* message);
    void appendFile(const char* path, const char* message);
    void logRowData(const char* message);
    void setup();

};

#endif // MEMORYCARDHANDLER_H

