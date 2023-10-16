#ifndef MEMORYCARDHANDLER_H
#define MEMORYCARDHANDLER_H

#include <string>
#include <vector>


class MemoryCard {

private:
    char NewFilePath[30];  // Increased the buffer size
public:
    const int memoryCardPin = 4;
    int PullLastEventIndex(fs::FS& fs, const char* path);
    void writeFile(fs::FS& fs, const char* path, const char* message);
    void appendFile(fs::FS& fs, const char* path, const char* message);
    void setup(memoryCardPin);
};

#endif // MEMORYCARDHANDLER_H

