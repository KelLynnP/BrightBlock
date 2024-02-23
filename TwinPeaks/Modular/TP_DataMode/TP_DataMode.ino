#include "GPSHandler.h"
#include "Button.h"
#include "LED.h"
#include "MemoryCardHandler.h"
#include "Sen55Handler.h"
#include "DataTranscription.h"
#include "StateMachine.h"
#include "LPS2.h"

// user Set configurations 
unsigned long sampleRate = 1500 ;
int DataLen = 11; 


// Object handlers
LPS2 lpsHandler;
GPSHandler gpsHandler(Serial1);
Sen55Handler sen55Handler;
MemoryCardHandler memoryCard;
DataTranscription Event;

// ButtonSet
Button* logEventButton;
uint8_t logEventButtonPin = 13;

Button* stateButton;
uint8_t stateButtonPin = 26;

// Led Vibes 
int brightHigh = 50;
int brightLow = 5;
int timeDelayMS = 1000; 

// Okay two helper functions, actually requires real logic change to make pull and transcribe data belong elsewhere.  
std::string makeString(float RawData, int length) {
    char Data[15];
    sprintf(Data, "%f", RawData);
    std::string str = Data;
    return str.substr(0,length);
}

std::vector<std::string> PullAndTranscribeData() {
  std::vector<std::string> sensorDataVector(DataLen);
  
  sen55Handler.pullData();
  lpsHandler.pullData();

  //GPS Data
  sensorDataVector[0] = gpsHandler.getFullTimeStamp();
  sensorDataVector[1] = makeString(gpsHandler.getLatitude(), 10);
  sensorDataVector[2] = makeString(gpsHandler.getLongitude(), 10);
  sensorDataVector[3] = makeString(gpsHandler.getAltitude(), 6);

  // SEN55 Data
  sensorDataVector[4] = makeString(sen55Handler.getPm2p5(), 6);
  
  
  sensorDataVector[5] = makeString(sen55Handler.getAmbientHumidity(), 6);
  sensorDataVector[6] = makeString(sen55Handler.getAmbientTemperature(), 6);
  sensorDataVector[7] = makeString(sen55Handler.getVocIndex(), 6);
  // Serial.println(sen55Handler.getVocIndex());

  sensorDataVector[8] = makeString(sen55Handler.getNoxIndex(), 6);
  
  // Log Button Data
  sensorDataVector[9] = makeString(logEventButton->getCount(), 6); 
  logEventButton->resetCount();
  
  // Pressure Sensor
  sensorDataVector[10] = makeString(lpsHandler.getPressureHPa(), 6); 

  return sensorDataVector;
}

LED StatusLED(25,0); // Create an instance of the LED class at pin 25 with channel 0

// State Machine Logic
StateMachine machine = StateMachine();
State* idle = machine.addState(&idleState);  // starting state~
State* dataTaking = machine.addState(&dataTakingState);

void setup() {
  Serial.begin(115200);
  gpsHandler.setup();
  sen55Handler.setup();
  memoryCard.setup();
  lpsHandler.setup();
  
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

// State machine functions
void idleState() {
  StatusLED.ledSet(0, 0, timeDelayMS);
}

void dataTakingState() {
  // Serial.printf("Data time!  \n");
  StatusLED.ledSet(brightHigh, brightLow, timeDelayMS);
  gpsHandler.readAndStoreGPS();

 
  if (millis() - Event.lastMillis > sampleRate) {  // Check every 5 seconds
    // Serial.printf("Checking Sample Rate : %lu %\n", millis() );

    std::vector<std::string> dataString = PullAndTranscribeData();
    std::string Row_Data;

    for (int i = 0; i < DataLen; i++) {
      Row_Data += dataString[i].c_str();
      
      if (i < DataLen-1){
        Row_Data += ',';
      }
    }
    Serial.println(Row_Data.c_str());
    Row_Data += '\n';
    memoryCard.logRowData(Row_Data.c_str()); // 

    Event.lastMillis = millis();
    // Serial.printf("Did this update : %.2ul %\n", Event.lastMillis );

  }
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
    memoryCard.setNewDataEvent();
    return true;
  }
  return false;

}

// old test function
void testByPrint(){
  StatusLED.ledSet(brightHigh, brightLow, timeDelayMS); 
  // gpsHandler.readAndStoreGPS();
  // static uint32_t lastMillis = 0;

  if (millis() - Event.lastMillis > sampleRate) {  // Check every 5 seconds
    Serial.printf("Temp: %lu reading [C]\n", millis());
    lpsHandler.pullData();
    // Serial.printf("Pressure reading: %2.2f reading [hPa]\n", lpsHandler.getPressureHPa());
    Serial.printf("Temp reading: %2.2f reading [C]\n", lpsHandler.getAmbientTemperature());

    Serial.printf("Log Button: %d times in the last [in 5 seconds]\n", logEventButton->getCount());

    Serial.printf("Log Button: %d times in the last [in 5 seconds]\n", stateButton->getCount());
    logEventButton->resetCount();
    stateButton->resetCount();

    // Serial.printf("Gps Data: %2.2f latitude \n", gpsHandler.getLatitude());
    // if (sen55Handler.pullData()){
    //   Serial.printf("Sen Data: %2.2f PM2.5 \n", sen55Handler.getPm2p5());
    //   }
    Event.lastMillis = millis();
  }
}
