#include "MemoryCardHandler.h"

int MemoryCardHandler::PullLastEventIndex(fs::FS& fs, const char* path) {  //FiXme: this calls other helper functions nested inside, i am okay with it because it helps declutter the already cluttered code below.
  Serial.println("Starting PullLastEventIndex ");

  File file = fs.open(path, FILE_READ);
  String line;
  if (!file) {
    Serial.println("Failed to open file for appending");
    return -1;
  }

  while (file.available()) {
    line = file.readStringUntil(',');
    Serial.println(line);
  }

  int numLine = std::stoi(line.c_str());
  Serial.printf("Appending to file: %s\n", path);
  std::string numLinePlusOne = std::to_string(numLine + 1);
  numLinePlusOne += ',';
  Serial.println(numLinePlusOne.c_str());
  file.close();
  appendFile(fs, path, numLinePlusOne.c_str());

  return numLine;
  // return 0;
}

void  MemoryCardHandler::writeFile(fs::FS& fs, const char* path, const char* message) {
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
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

void  MemoryCardHandler::appendFile(fs::FS& fs, const char* path, const char* message) {
  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if (!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  if (file.print(message)) {
    Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
  file.close();
}