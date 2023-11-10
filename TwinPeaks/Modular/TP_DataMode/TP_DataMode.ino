#include "GPSHandler.h"
#include "Button.h"
#include "LED.h"
#include "MemoryCardHandler.h"
#include "Sen55Handler.h"
#include "DataTranscription.h"
#include <StateMachine.h>

// Object handlers
GPSHandler gpsHandler(Serial1);
Sen55Handler sen55Handler;
MemoryCardHandler memoryCard;


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

// State Machine Logic
StateMachine machine = StateMachine();
State* idle = machine.addState(&idleState);  // starting state~
State* dataTaking = machine.addState(&dataTakingState);

void setup() {
  Serial.begin(115200);
  gpsHandler.setup();
  sen55Handler.setup();
  
  logEventButton = new Button();
  logEventButton->setup(logEventButtonPin, []{ logEventButton->handleInterrupt(); }, RISING);

  stateButton = new Button();
  stateButton->setup(stateButtonPin, []{ stateButton->handleInterrupt(); }, RISING);

  // set transtiions
  idle->addTransition(&transitionIdle2dataTaking, dataTaking);
  dataTaking->addTransition(&transitiondataTaking2Idle, idle);  
}

void loop() {
   machine.run();
  // testByPrint();
}
  
void idleState() {
  // Serial.printf("Idle State ! Nothing 2 c here :) \n");
  StatusLED.ledSet(0, 0, timeDelayMS);
}

void dataTakingState() {
  // Serial.printf("Data time!  \n");
  StatusLED.ledSet(brightHigh, brightLow, timeDelayMS);
}

bool transitiondataTaking2Idle() {
  int buttonCountForPrint = stateButton->getCount();
  if (buttonCountForPrint != 0) {
    Serial.printf("Beeep Boop time to swap state! Data Taking -> Idle \n");
    stateButton->resetCount();
    return true;
  }
  return false;
}

bool transitionIdle2dataTaking() {
  int buttonCountForPrint = stateButton->getCount();
  if (buttonCountForPrint != 0) {
    Serial.printf("Boop Beep time to swap state! Idle -> Data Taking  \n");
    stateButton->resetCount();
    return true;
  }
  return false;
}

void testByPrint(){
  StatusLED.ledSet(brightHigh, brightLow, timeDelayMS); 
  gpsHandler.readAndStoreGPS();

  static uint32_t lastMillis = 0;

  if (millis() - lastMillis > 5000UL) {  // Check every 5 seconds
    Serial.printf("Log Button: %d times in the last [in 5 seconds]\n", logEventButton->getCount());
    Serial.printf("State Button: %d times in the last [in 5 seconds]\n", stateButton->getCount());
    Serial.printf("Gps Data: %2.2f latitude \n", gpsHandler.getLatitude());
    if (sen55Handler.pullData()){
      Serial.printf("Sen Data: %2.2f PM2.5 \n", sen55Handler.getPm2p5());
      }
    lastMillis = millis();
  }
}


