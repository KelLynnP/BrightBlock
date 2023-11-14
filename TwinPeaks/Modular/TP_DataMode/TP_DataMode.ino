#include "GPSHandler.h"
#include "Button.h"
#include "LED.h"
#include "MemoryCardHandler.h"
#include "Sen55Handler.h"
#include "DataTranscription.h"
#include "StateMachine.h"
#include "FS.h"  // Memory Card files
#include "SD.h"
#include "SPI.h"

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

std::string makeString(float RawData, int length) {
    char Data[15];
    sprintf(Data, "%f", RawData);
    // Serial.print(Data);
    std::string str = Data;
    return str.substr(0,length);
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
  
  logEventButton = new Button();
  logEventButton->setup(logEventButtonPin, []{ logEventButton->handleInterrupt(); }, RISING);

  stateButton = new Button();
  stateButton->setup(stateButtonPin, []{ stateButton->handleInterrupt(); }, RISING);

  // set transtiions
  idle->addTransition(&transitionIdle2dataTaking, dataTaking);
  dataTaking->addTransition(&transitiondataTaking2Idle, idle);  
  static uint32_t lastMillis = 0;
}

void loop() {
   machine.run();
  // testByPrint();
}

void idleState() {
  StatusLED.ledSet(0, 0, timeDelayMS);
}

std::vector<std::string> PullAndTranscribeData() {
    std::vector<std::string> sensorDataVector(10);
    
    sen55Handler.pullData();

    //GPS Data
    sensorDataVector[0] = gpsHandler.getFullTimeStamp();
    sensorDataVector[1] = makeString(gpsHandler.getLatitude() ,6);
    sensorDataVector[2] = makeString(gpsHandler.getLongitude(), 6);
    sensorDataVector[3] = makeString(gpsHandler.getAltitude(), 6);

    // SEN55 Data
    sensorDataVector[4] = makeString(sen55Handler.getPm2p5(), 6);
    sensorDataVector[5] = makeString(sen55Handler.getAmbientHumidity(), 6);
    sensorDataVector[6] = makeString(sen55Handler.getAmbientTemperature(), 6);
    sensorDataVector[7] = makeString(sen55Handler.getVocIndex(), 6);
    sensorDataVector[8] = makeString(sen55Handler.getNoxIndex(), 6);
    
    // Log Button Data
    sensorDataVector[9] = makeString(logEventButton->getCount(), 6); 
    logEventButton->resetCount();

  return sensorDataVector;
}

void dataTakingState() {
  // Serial.printf("Data time!  \n");
  StatusLED.ledSet(brightHigh, brightLow, timeDelayMS);
  gpsHandler.readAndStoreGPS();
  static uint32_t lastMillis = 0;

  if (millis() - lastMillis > 5000UL) {  // Check every 5 seconds
    std::vector<std::string> dataString = PullAndTranscribeData();
    std::string Row_Data;

    for (int i = 0; i < 10; i++) {
      Row_Data += dataString[i].c_str();
      Row_Data += ',';
    }
    Serial.println(Row_Data.c_str());

    Row_Data += '\n';
    memoryCard.logRowData(Row_Data.c_str()); // 

    lastMillis = millis();
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


void testByPrint(){
  StatusLED.ledSet(brightHigh, brightLow, timeDelayMS); 
  gpsHandler.readAndStoreGPS();
  static uint32_t lastMillis = 0;

  if (millis() - lastMillis > 5000UL) {  // Check every 5 seconds
    Serial.printf("Log Button: %d times in the last [in 5 seconds]\n", logEventButton->getCount());

    logEventButton->resetCount();
    Serial.printf("Log Button: %d times in the last [in 5 seconds]\n", logEventButton->getCount());

    Serial.printf("Gps Data: %2.2f latitude \n", gpsHandler.getLatitude());
    if (sen55Handler.pullData()){
      Serial.printf("Sen Data: %2.2f PM2.5 \n", sen55Handler.getPm2p5());
      }
    lastMillis = millis();
  }
}

std::string FormatAndAppendTimestamp(float RawData, const char* TimeSnip) {
  char Data[15];
  sprintf(Data, ",%f", RawData);
  std::string str = std::string(TimeSnip) + Data;
  return str;
}

std::vector<std::string> PullAndTranscribeDataEncodedWithTimeStamp() {
  //std::to_string(myFloat);
  std::vector<std::string> sensorDataVector(10);

  //GPS Data
  sensorDataVector[0] = gpsHandler.getFullTimeStamp();
  sensorDataVector[1] = FormatAndAppendTimestamp(gpsHandler.getLatitude(), gpsHandler.getShortTimeStamp());
  sensorDataVector[2] = FormatAndAppendTimestamp(gpsHandler.getLongitude(),gpsHandler.getShortTimeStamp());
  sensorDataVector[3] = FormatAndAppendTimestamp(gpsHandler.getAltitude(), gpsHandler.getShortTimeStamp());
  // SEN55 Data
  sensorDataVector[4] = FormatAndAppendTimestamp(sen55Handler.getPm2p5(),gpsHandler.getShortTimeStamp());
  sensorDataVector[5] = FormatAndAppendTimestamp(sen55Handler.getAmbientHumidity(), gpsHandler.getShortTimeStamp());
  sensorDataVector[6] = FormatAndAppendTimestamp(sen55Handler.getAmbientTemperature(), gpsHandler.getShortTimeStamp());
  // sensorDataVector[7] = FormatAndAppendTimestamp(sen55Handler.getVocIndex, gpsHandler.getShortTimeStamp());
  // sensorDataVector[8] = FormatAndAppendTimestamp(sen55Handler.getNoxIndex, gpsHandler.getShortTimeStamp());
  
  // Log Button Data
  sensorDataVector[9] = FormatAndAppendTimestamp(logEventButton->getCount(), gpsHandler.getShortTimeStamp());
  logEventButton->resetCount();

  return sensorDataVector;
}
