#ifndef MEMORYCARDHANDLER_H
#define MEMORYCARDHANDLER_H

#include <string>
#include <vector>
#include "FS.h"  // Memory Card files
#include "SD.h"
#include "SPI.h"


class MemoryCardHandler {

private:
    char NewFilePath[30];  // Increased the buffer size
    int eventCounter = -1;
    const char eventIndexPath[30] = "/eventIndex.txt"; 
    fs::FS &sd = SD; // Declare SD as a member variable

    
public:
    int memoryCardPin = 4;
    int PullLastEventIndex();
    // int PullLastEventIndex(fs::FS& fs, const char* path);
    void setNewDataEvent();
    void writeFile(const char* path, const char* message);
    void appendFile(const char* path, const char* message);
    void setup();
};

#endif // MEMORYCARDHANDLER_H

