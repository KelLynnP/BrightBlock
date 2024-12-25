#include "../../handlers/memoryCardHandler.h"
#include "../../handlers/memoryCardHandler.cpp"

#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include "FS.h" 

// Define the pins we want to use
static const int CS = 4;
static const int CUSTOM_SCK = 5;    // Your preferred SCK pin
static const int CUSTOM_MOSI = 18;  // Your preferred MOSI pin
static const int CUSTOM_MISO = 19;  // Your preferred MISO pin

void setup() {
    Serial.begin(115200);
    
    // Debug print the default pins
    Serial.printf("Default MOSI: %d\n", MOSI);
    Serial.printf("Default MISO: %d\n", MISO);
    Serial.printf("Default SCK: %d\n", SCK);
    
    pinMode(CUSTOM_MISO, INPUT_PULLDOWN); 
    pinMode(CUSTOM_MOSI, OUTPUT);
    pinMode(CUSTOM_SCK, OUTPUT);
    pinMode(CS, OUTPUT);

    // Initialize SPI with our custom pins
    SPI.begin(CUSTOM_SCK, CUSTOM_MISO, CUSTOM_MOSI, CS);  
    
    if (!SD.begin(CS)) {
        Serial.println("Card Mount Failed");
        return;
    }
    Serial.println("Successfully Mounted Card");

}

void loop() {
    delay(1000);
}