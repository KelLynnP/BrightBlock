#include "MemoryCardHandler.h"
#include "FS.h"  // Memory Card files
#include "SD.h"
#include "SPI.h"

void MemoryCardHandler::setup() {
    if (!SD.begin(memoryCardPin)) {
        Serial.println("Card Mount Failed");
        return;
    }
  uint8_t cardType = SD.cardType();

  if (cardType == CARD_NONE) {
    Serial.println("No SD card attached");
    return;
  }

  Serial.print("SD Card Type: ");
  if (cardType == CARD_MMC) {
    Serial.println("MMC");
  } else if (cardType == CARD_SD) {
    Serial.println("SDSC");
  } else if (cardType == CARD_SDHC) {
    Serial.println("SDHC");
  } else {
    Serial.println("UNKNOWN");
  }
  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
}

void MemoryCardHandler::setNewDataEvent(){
   int eventCounter = PullLastEventIndex();
    sprintf(NewFilePath, "/Event%d.txt", eventCounter);  // Fixed format string
    Serial.print("New data file created");
    Serial.println(NewFilePath);
    String Header = "TimeStamp,Latitude,Longitude,Altitude,PM25,RelativeHumidity,Temperature,vocIndex,noxIndex,ButtonPress,PressurehPa\n";
    writeFile(NewFilePath, Header.c_str());  
}

int MemoryCardHandler::PullLastEventIndex() { 
  Serial.println("Starting PullLastEventIndex ");
  File file = SD.open(MemoryCardHandler::eventIndexPath, FILE_READ);
  String line;
  if (!file) {
    Serial.println("Failed to open file for appending");
    return -1;
  }

  while (file.available()) {
    line = file.readStringUntil(',');
    Serial.println(line);
  }
  int numLine = 0;
  try {
      numLine = std::stoi(line.c_str());
  } catch (const std::exception& e) {
      Serial.println("Error converting string to integer");
      Serial.println(e.what());
  }

  std::string numLinePlusOne = std::to_string(numLine + 1);
  numLinePlusOne += ',';
   if (printOut){
    Serial.printf("Appending to file: %s\n", MemoryCardHandler::eventIndexPath);
    Serial.println(numLinePlusOne.c_str());
   }
  file.close();
  appendFile(MemoryCardHandler::eventIndexPath, numLinePlusOne.c_str());
  return numLine;
}

void  MemoryCardHandler::writeFile(const char* path, const char* message) {
  Serial.printf("Writing file: %s\n", path);

  File file = SD.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  file.close();
}

void  MemoryCardHandler::appendFile(const char* path, const char* message) {
  if (printOut){
    Serial.printf("Appending to file: %s\n", path);
  }


  File file = SD.open(path, FILE_APPEND);
  if (!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  if (file.print(message) & (printOut)){
    Serial.println("Message appended");
  } if  (!file.print(message)){
    Serial.println("Append failed");
  }
  file.close();
}

void MemoryCardHandler::logRowData(const char* message){
  appendFile(NewFilePath, message);  // Corrected the function name to appendFile and added missing ".c_str()"
}

