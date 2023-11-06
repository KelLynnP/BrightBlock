#include "GPSHandler.h"
#include "Button.h"
#include "LED.h"
#include "MemoryCardHandler.h"
#include "Sen55Handler.h"

// const char* UUIDLabels[];
// const char* characteristicUUIDs[];
// const int NumCharacteristics;

// GPS handler
 GPSHandler gpsHandler(Serial1);
 Sen55Handler sen55Handler;

// ButtonSet
 Button* logEventButton;
 uint8_t logEventButtonPin = 13;

 Button* stateButton;
 uint8_t stateButtonPin = 26;

// Led Vibes 
 int brightHigh = 30;
 int brightLow = 5;
 int timeDelayMS = 1000; 

 LED StatusLED(25,0); // Create an instance of the LED class at pin 25 with channel 0

void setup() {
    Serial.begin(115200);
    gpsHandler.setup();
    sen55Handler.setup();


    logEventButton = new Button();
    logEventButton->setup(logEventButtonPin, []{ logEventButton->handleInterrupt(); }, RISING);

    stateButton = new Button();
    stateButton->setup(stateButtonPin, []{ stateButton->handleInterrupt(); }, RISING);
}

void loop() {
  StatusLED.ledSet(brightHigh, brightLow, timeDelayMS); 
  gpsHandler.readAndStoreGPS();


  static uint32_t lastMillis = 0;

  if (millis() - lastMillis > 5000UL) {  // Check every 5 seconds
    
    Serial.printf("Log Button: %d times in the last [in 5 seconds]\n", logEventButton->getCount());
    Serial.printf("State Button: %d times in the last [in 5 seconds]\n", stateButton->getCount());
    Serial.printf("Gps Data: %2.2f latitude \n", gpsHandler.getLatitude());
    Serial.printf("Sen Data: %2.2f latitude \n", sen55Handler.getPm2p5());
    lastMillis = millis();
  }
}
