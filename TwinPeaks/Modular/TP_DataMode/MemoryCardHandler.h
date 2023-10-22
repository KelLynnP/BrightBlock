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
public:
    int memoryCardPin = 4;
    int PullLastEventIndex(fs::FS& fs, const char* path);
    void writeFile(fs::FS& fs, const char* path, const char* message);
    void appendFile(fs::FS& fs, const char* path, const char* message);
    void setup(int memoryCardPin);
};

#endif // MEMORYCARDHANDLER_H

