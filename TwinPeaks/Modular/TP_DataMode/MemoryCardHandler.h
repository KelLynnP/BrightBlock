#ifndef MEMORYCARDHANDLER_H
#define MEMORYCARDHANDLER_H

#include <string>
#include <vector>
#include "FS.h"  // Memory Card files
#include "SD.h"
#include "SPI.h"


class MemoryCardHandler {

private:
    int eventCounter = -1;
    const char eventIndexPath[30] = "/eventIndex.txt"; 
    char NewFilePath[30];  // Increased the buffer size

    
public:
    bool printOut = false;
    int memoryCardPin = 4;
    int PullLastEventIndex();
    void setNewDataEvent();
    void writeFile(const char* path, const char* message);
    void appendFile(const char* path, const char* message);
    void logRowData(const char* message);
    void setup();

};

#endif // MEMORYCARDHANDLER_H

