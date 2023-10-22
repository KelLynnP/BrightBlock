#include "GPSHandler.h"
#include "Button.h"
#include "LED.h"

// const char* UUIDLabels[];
// const char* characteristicUUIDs[];
// const int NumCharacteristics;

// GPS handler
GPSHandler gpsHandler(Serial1);

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

    logEventButton = new Button();
    logEventButton->setup(logEventButtonPin, []{ logEventButton->handleInterrupt(); }, RISING);

    stateButton = new Button();
    stateButton->setup(stateButtonPin, []{ stateButton->handleInterrupt(); }, RISING);
}

void loop() {
    StatusLED.ledSet(brightHigh, brightLow, timeDelayMS); 

    GPSHandler::GPSData data = gpsHandler.readAndStoreGPS();
    if (data.dataReceived) {
        // Process the data

    }

  static uint32_t lastMillis = 0;

  if (millis() - lastMillis > 5000UL) {  // Check every 5 seconds
    
    Serial.printf("Log Button: %d times in the last [in 5 seconds]\n", logEventButton->getCount());
    Serial.printf("State Button: %d times in the last [in 5 seconds]\n", stateButton->getCount());

    lastMillis = millis();
  }
}
