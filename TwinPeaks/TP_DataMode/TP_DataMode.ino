// Libraries 
  #include <NimBLEDevice.h>
  #include <unordered_map>

  #include <Adafruit_GPS.h>
  #include <Arduino.h>  // Sensirion I2C libraries (lol @ not using arduino?)
  #include <SensirionI2CSen5x.h>
  #include <Wire.h>
  #include <driver/ledc.h>
  #include <iostream>
  #include <string>

  #include "FS.h"  // Memory Card files
  #include "SD.h"
  #include "SPI.h"

  #include <ArduinoJson.h>
  #include <unordered_map>
  #include <SoftwareSerial.h>
  #include <vector>
  #include <StateMachine.h>
// End Libraries 

// BUTTON STUFF
// class Button {
// public:
//   void setup(uint8_t irq_pin, void (*ISR_callback)(void), int value);
//   void handleInterrupt(void);
//   uint32_t getCount();
//   void resetCount();

// private:
//   uint32_t countNumKeyPresses = 0;
//   int debounceTime = 250;  // ms
//   int timeSinceLastPress = 0;
//   int rightNowButtonPressTime = 0;
//   bool boolPrint = true;  // Set a default value
// };

// void Button::setup(uint8_t irq_pin, void (*ISR_callback)(void), int value) {
//   pinMode(irq_pin, INPUT_PULLUP);
//   attachInterrupt(digitalPinToInterrupt(irq_pin), ISR_callback, value);
//   Serial.println("Button Initialized");
// }


// void Button::handleInterrupt(void) {
//   rightNowButtonPressTime = millis();
//   if (rightNowButtonPressTime - timeSinceLastPress >= debounceTime) {
//     countNumKeyPresses++;
//     timeSinceLastPress = rightNowButtonPressTime;
//   }
// }
// uint32_t Button::getCount() {
//   return countNumKeyPresses;
// }
// void Button::resetCount() {
//   countNumKeyPresses = 0;
// }

// // LED STUFF

// class LED {
// private: 
//     std::string state;
//     uint32_t LEDC_CHANNEL_0;  // #FIXME -> channels can change (dig plz)
//     uint32_t LEDC_TIMER_12_BIT = 12; 
//     uint32_t LEDC_BASE_FREQ; 
//     uint32_t lastToggleTime; 

// public:
//     int LedPin;
//     LED(int LedPinAssigned, int ChannelSet): LedPin(LedPinAssigned), LEDC_CHANNEL_0(ChannelSet) {
//       LEDC_BASE_FREQ = 5000; // use 5000 Hz as a LEDC base frequency// could change based on LED
//       lastToggleTime = millis(); 
//       state = "low"; 
//       ledcSetup(LEDC_CHANNEL_0, LEDC_BASE_FREQ, LEDC_TIMER_12_BIT);
//       ledcAttachPin(LedPinAssigned, LEDC_CHANNEL_0);
//     }

//     void ledSet(int brightnessHigh, int brightnessLow, int timingInterval){ // neeed to add flash frequency
//       if ((millis() - lastToggleTime) >= timingInterval) {
//         if (state == "low"){
//           ledcAnalogWrite(LEDC_CHANNEL_0, brightnessHigh);
//           state = "high"; 
//           // Serial.println("Writing High ");
//         }else{
//           ledcAnalogWrite(LEDC_CHANNEL_0, brightnessLow); 
//           state = "low";
//           // Serial.println("Writing Low ");
//         }
//         lastToggleTime = millis();
//       }
//     }

//     void ledcAnalogWrite(uint8_t channel, uint32_t value) {
//       // calculate duty, 4095 from 2 ^ 12 - 1
//       uint32_t valueMax = 255;
//       uint32_t duty = (4095 / valueMax) * min(value, valueMax);
//       // write duty to LEDC
//       ledcWrite(channel, duty); 
//     }
// };

// ---------------------------------------------------------------//
// ------------------ User input Info ----------------------------//
// ---------------------------------------------------------------//

const char* deviceName = "BLEEP";
const unsigned long sampleRate = 1000;

// Which Sensors

const char* UUIDLabels[] = {
  "TimeStamp",         // From GPS
  "Latitude",          // From GPS
  "Longitude",         // From GPS
  "Altitude",          // From GPS
  "PM25",              // From SEN55
  "RelativeHumidity",  // From SEN55
  "Temperature",       // From SEN55
  "vocIndex",          // From SEN55
  "noxIndex",          // From SEN55
  "PM10",              // From SEN55
  "ButtonPress"        // From Input button
};

const int NumCharacteristics = sizeof(UUIDLabels) / sizeof(UUIDLabels[0]);

const char* characteristicUUIDs[] = {
  "beb5483e-36e1-4688-b7f5-ea07361b26a8",
  "1c95d5e3-d8f7-413a-bf3d-7a2e5d7be87e",
  "d7d85823-5304-4eb3-9671-3e571fac07b9",
  "d2789cef-106f-4660-9e3f-584c12e2e3c7",
  "bf5a799d-26d0-410e-96b0-9ada1eb9f758",
  "c22b405e-2b7b-4632-831d-54523e169a01",
  "ffdda8ad-60a2-4184-baff-5c79a2eccb8c",
  "183b971a-79f5-4004-8182-31c88d910dca",
  "90b77f62-003d-454e-97fc-8f597b42048c",
  "86cef02b-8c15-457b-b480-52e6cc0bdd8c",
  "755c7c73-b938-4a6e-a7be-2a3b8c3783d9",
};

// ---------------------------------------------------------------//
// ------------------ GPS information ----------------------------//
// ---------------------------------------------------------------//

// Could be re-written as a class vs a struct for cleaner code (#Fight me :))
#define GPSSerial Serial1
Adafruit_GPS GPS(&GPSSerial);
#define GPSECHO false
uint32_t GpsTimer = millis();
uint32_t GlobalTimer = millis();
const int sampleRateGps = 5000;  // update line in initiation if this changes

struct GPSData {
  char FullTimeStamp[20];
  char ShortTimeStamp[6];
  float latitude;
  float longitude;
  float altitude;
  bool dataReceived = false;  // set true if set correctly
};

GPSData readAndStoreGPS() {
  GPSData TempGPS;
  char c = GPS.read();  // BECAUSE OF GPS LOGIC THIS NEEDS TO BE CALLED AT LEAST TWICE A SECOND... This means global delay functions are OUT   // Serial.print(c); // SHOWS ALL THE NMEA STRINGS! PRINT AT UR OWN RISK :)
  // Serial.println(c);
  if (GPSECHO) {
    if (c) {
      Serial.print(c);  // if a sentence is received, we can check the checksum, parse it...
    }
  }

  if (GPS.newNMEAreceived()) {       // a tricky thing here is if we print the NMEA sentence, or data // we end up not listening and catching other sentences! // so be very wary if using OUTPUT_ALLDATA and trying to print out data     // Serial.print(GPS.lastNMEA()); // this also sets the newNMEAreceived() flag to false
    if (!GPS.parse(GPS.lastNMEA()))  // this also sets the newNMEAreceived() flag to fals
      return TempGPS;
  }

  if ((millis() - GpsTimer > sampleRateGps) && (!GPS.parse(GPS.lastNMEA())) && (GPS.fix != 0)) {
    GpsTimer = millis();  // reset the timer
    sprintf(TempGPS.FullTimeStamp, "%02d:%02d:%02d %02d/%02d/%04d", GPS.hour, GPS.minute, GPS.seconds, GPS.day, GPS.month, GPS.year);
    sprintf(TempGPS.ShortTimeStamp, "%02d%02d%02d", GPS.hour, GPS.minute, GPS.seconds);
    TempGPS.latitude = GPS.latitudeDegrees;
    TempGPS.longitude = GPS.longitudeDegrees;
    TempGPS.altitude = GPS.altitude;
    TempGPS.dataReceived = true;
    return TempGPS;
  } else {  // settting up a Dummy vector    // Serial.print((!GPS.parse(GPS.lastNMEA())));
    GPSData TempGPS;
    sprintf(TempGPS.FullTimeStamp, "%02d:%02d:%02d %02d/%02d/%04d", 90, 90, 90, 90, 90, 1999);
    sprintf(TempGPS.ShortTimeStamp, "%02d%02d%02d", 90, 90, 90);
    TempGPS.latitude = -1;
    TempGPS.longitude = -1;
    TempGPS.altitude = -1;
    TempGPS.dataReceived = false;
    return TempGPS;
  }
}
// End GPS Comment

// ---------------------------------------------------------------//
// ------------------ BLE information ----------------------------//
// ---------------------------------------------------------------//



static NimBLEUUID BLESERVICE_UUID("4fafc201-1fb5-459e-8fcc-c5c9c331914b");
bool deviceConnected = false;
bool oldDeviceConnected = false;
static NimBLEServer* pServer;

class MyServerCallbacks : public NimBLEServerCallbacks {

  void onConnect(NimBLEServer* pServer) override {
    deviceConnected = true;
  };

  void onDisconnect(NimBLEServer* pServer) override {
    deviceConnected = false;
  };
};

NimBLECharacteristic* intializeBLECharacteristic(NimBLECharacteristic* pCharacteristic, NimBLEService* pService, const char* characteristicUUID, const char* label) {
  pCharacteristic = pService->createCharacteristic(
    characteristicUUID,
    NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::READ);

  pCharacteristic->createDescriptor("2901", NIMBLE_PROPERTY::READ)->setValue(label);

  return pCharacteristic;
}

NimBLECharacteristic* pCharacteristicChars[NumCharacteristics] = { NULL };
// OLD BLE CODE

  // Initialized Once // Not Characterisitic Specific
  // static BLEUUID BLESERVICE_UUID("4fafc201-1fb5-459e-8fcc-c5c9c331914b");
  // const int numServices = 100;  //#FIXME: need to way expand past number of UUIDS
  // bool deviceConnected = false;
  // bool oldDeviceConnected = false;
  // BLEServer* pServer = NULL;  // Pointer to the server
  // Characteristic Specific
  // BLEDescriptor* pDescr;  // Pointer to Descriptor of Characteristic 1
  // BLE2902* pBLE2902;      // Pointer to BLE2902 of Characteristic 1
  // BLECharacteristic* pCharacteristicChars[NumCharacteristics] = { NULL };

  // class MyServerCallbacks : public BLEServerCallbacks {
  //   void onConnect(BLEServer* pServer) {
  //     deviceConnected = true;
  //   };

  //   void onDisconnect(BLEServer* pServer) {
  //     deviceConnected = false;
  //   };
  // };

  // // Function to set up all the Characteristics
  // BLECharacteristic* intializeBLECharacteristic(BLECharacteristic* pCharacteristic, BLEService* pService, const char* characteristicUUID, const char* label) {
  //   // 2.a. Characteristics Here
  //   pCharacteristic = pService->createCharacteristic(
  //     characteristicUUID,
  //     BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_READ);
  //   // BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY);

  //   // 2.b.i Create a BLE Descriptor
  //   pDescr = new BLEDescriptor((uint16_t)0x2901);  //The 0x2901 is the Bluetooth SIG-defined UUID for the "Characteristic User Description" descriptor. This descriptor is used to provide a human-readable description or name for the characteristic.
  //   Serial.print(characteristicUUID);
  //   pDescr->setValue(label);
  //   pCharacteristic->addDescriptor(pDescr);

  //   //2.b.ii Add the BLE2902 Descriptor because we are using "PROPERTY_NOTIFY"
  //   pBLE2902 = new BLE2902();
  //   pBLE2902->setNotifications(true);
  //   pCharacteristic->addDescriptor(pBLE2902);
  //   return pCharacteristic;
  // }
// End BLE Comment


// ---------------------------------------------------------------//
// ------------------ Memory Card Stuff --------------------------//
// ---------------------------------------------------------------//

// Memory Card functions + inputs
const boolean isMemoryCardAttached = true;
const int memoryCardPin = 4;
int eventCounter = -1;
bool isEventStarted = false;
const char eventIndexPath[] = "/eventIndex.txt";
char NewFilePath[30];

int PullLastEventIndex(fs::FS& fs, const char* path) {  //FiXme: this calls other helper functions nested inside, i am okay with it because it helps declutter the already cluttered code below.
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

void writeFile(fs::FS& fs, const char* path, const char* message) {
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

void appendFile(fs::FS& fs, const char* path, const char* message) {
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
// Pull last event index, write file, append file

// ---------------------------------------------------------------//
// ----------- Data Mode + Transition Specific Work  -------------//
// ---------------------------------------------------------------//

  // State machine + UI ( Pin mode + button )
  StateMachine machine = StateMachine();

  State* idle = machine.addState(&idleState);  // starting state~
  State* dataTaking = machine.addState(&dataTakingState);
  // State* dataDump = machine.addState(&dataDumpState);

  // // ButtonSet
  // Button* logEventButton;
  // uint8_t logEventButtonPin = 12;

  // Button* stateButton;
  // uint8_t stateButtonPin = 26;

  // LED StatusLED(26,0); //okay make an instance of this guy @pin 25, keeping channel @0
  int brightHigh = 10;
  int brightLow = 5;
  int timeDelayMS = 1000;
  int channel0 = 0;
  int ledPin = 25;

  LED StatusLED(ledPin, channel0);  //okay make an instance of this guy @pin 25, keeping channel @0
                                  // done

// ---------------------------------------------------------------//
// -------- Sensor Declarations + Helper Functions ---------------//
// ---------------------------------------------------------------//

// Sensirion Data things The used commands use up to 48 bytes. On some Arduino's the default buffer space is not large enough
#define MAXBUF_REQUIREMENT 48

#if (defined(I2C_BUFFER_LENGTH) && (I2C_BUFFER_LENGTH >= MAXBUF_REQUIREMENT)) || (defined(BUFFER_LENGTH) && BUFFER_LENGTH >= MAXBUF_REQUIREMENT)
#define USE_PRODUCT_INFO
#endif

SensirionI2CSen5x sen5x;
// both the following are decently useless and I would love to abstract them away :)
void printModuleVersions() {
  uint16_t error;
  char errorMessage[256];

  unsigned char productName[32];
  uint8_t productNameSize = 32;

  error = sen5x.getProductName(productName, productNameSize);

  if (error) {
    Serial.print("Error trying to execute getProductName(): ");
    errorToString(error, errorMessage, 256);
    Serial.println(errorMessage);
  } else {
    Serial.print("ProductName:");
    Serial.println((char*)productName);
  }

  uint8_t firmwareMajor;
  uint8_t firmwareMinor;
  bool firmwareDebug;
  uint8_t hardwareMajor;
  uint8_t hardwareMinor;
  uint8_t protocolMajor;
  uint8_t protocolMinor;

  error = sen5x.getVersion(firmwareMajor, firmwareMinor, firmwareDebug,
                           hardwareMajor, hardwareMinor, protocolMajor,
                           protocolMinor);
  if (error) {
    Serial.print("Error trying to execute getVersion(): ");
    errorToString(error, errorMessage, 256);
    Serial.println(errorMessage);
  } else {
    Serial.print("Firmware: ");
    Serial.print(firmwareMajor);
    Serial.print(".");
    Serial.print(firmwareMinor);
    Serial.print(", ");

    Serial.print("Hardware: ");
    Serial.print(hardwareMajor);
    Serial.print(".");
    Serial.println(hardwareMinor);
  }
}

void printSerialNumber() {
  uint16_t error;
  char errorMessage[256];
  unsigned char serialNumber[32];
  uint8_t serialNumberSize = 32;

  error = sen5x.getSerialNumber(serialNumber, serialNumberSize);
  if (error) {
    Serial.print("Error trying to execute getSerialNumber(): ");
    errorToString(error, errorMessage, 256);
    Serial.println(errorMessage);
  } else {
    Serial.print("SerialNumber:");
    Serial.println((char*)serialNumber);
  }
}

std::string FormatAndAppendTimestamp(float RawData, const char* TimeSnip) {
  char Data[15];  // number of bytes transferable!
  // Serial.println(RawData);
  sprintf(Data, ",%f", RawData);
  std::string str = std::string(TimeSnip) + Data;
  Serial.println(str.c_str());
  return str;
}

std::vector<std::string> PullAndTranscribeData(const GPSData& GPSData2Transmit) {
  std::vector<std::string> sensorDataVector(NumCharacteristics);

  // Read Measurement SEN 55
  char errorMessage[256];
  uint16_t error;

  float massConcentrationPm1p0;
  float massConcentrationPm2p5;
  float massConcentrationPm4p0;
  float massConcentrationPm10p0;
  float ambientHumidity;
  float ambientTemperature;
  float vocIndex;
  float noxIndex;

  error = sen5x.readMeasuredValues(
    massConcentrationPm1p0, massConcentrationPm2p5, massConcentrationPm4p0,
    massConcentrationPm10p0, ambientHumidity, ambientTemperature, vocIndex,
    noxIndex);

  if (error) {
    Serial.print("Error trying to execute readMeasuredValues(): ");
    errorToString(error, errorMessage, 256);
    Serial.println(errorMessage);
  }

  // GPS Data All pulled seperatly
  sensorDataVector[0] = GPSData2Transmit.FullTimeStamp;
  sensorDataVector[1] = FormatAndAppendTimestamp(GPSData2Transmit.latitude, GPSData2Transmit.ShortTimeStamp);
  sensorDataVector[2] = FormatAndAppendTimestamp(GPSData2Transmit.longitude, GPSData2Transmit.ShortTimeStamp);
  sensorDataVector[3] = FormatAndAppendTimestamp(GPSData2Transmit.altitude, GPSData2Transmit.ShortTimeStamp);

  // SEN 55 Data
  sensorDataVector[4] = FormatAndAppendTimestamp(massConcentrationPm2p5, GPSData2Transmit.ShortTimeStamp);
  sensorDataVector[5] = FormatAndAppendTimestamp(ambientHumidity, GPSData2Transmit.ShortTimeStamp);
  sensorDataVector[6] = FormatAndAppendTimestamp(ambientTemperature, GPSData2Transmit.ShortTimeStamp);
  sensorDataVector[7] = FormatAndAppendTimestamp(vocIndex, GPSData2Transmit.ShortTimeStamp);
  sensorDataVector[8] = FormatAndAppendTimestamp(noxIndex, GPSData2Transmit.ShortTimeStamp);
  sensorDataVector[9] = FormatAndAppendTimestamp(massConcentrationPm10p0, GPSData2Transmit.ShortTimeStamp);

  // Log Button Data
  sensorDataVector[10] = FormatAndAppendTimestamp(logEventButton->getCount(), GPSData2Transmit.ShortTimeStamp);
  logEventButton->resetCount();

  return sensorDataVector;
}
// Sensirion Data

void setup() {
  Serial.begin(115200);

  //----------- Memory Card Begin ------------- //
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

  // set up sensors (GPS + Sensirion)

  //----------- GPS Set up-------------- //
  GPSSerial.begin(9600);  // #fixme (should I be worried about this?)
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_5HZ);  // 1 Hz update rate

  //---------- PM 2.5 Set Up ------------ //
  Wire.begin();
  sen5x.begin(Wire);
  char errorMessage[256];
  uint16_t error;
  error = sen5x.deviceReset();
  if (error) {
    Serial.print("Error trying to execute deviceReset(): ");
    errorToString(error, errorMessage, 256);
    Serial.println(errorMessage);
  }

  // Print SEN55 module information if i2c buffers are large enough
  // #ifdef USE_PRODUCT_INFO
  //   printSerialNumber();
  //   printModuleVersions();
  // #endif

  // Look at temp offset info in Notion
  float tempOffset = 0.0;
  error = sen5x.setTemperatureOffsetSimple(tempOffset);
  if (error) {
    Serial.print("Error trying to execute setTemperatureOffsetSimple(): ");
    errorToString(error, errorMessage, 256);
    Serial.println(errorMessage);
  } else {
    Serial.print("Temperature Offset set to ");
    Serial.print(tempOffset);
    Serial.println(" deg. Celsius (SEN54/SEN55 only");
  }

  // Start Measurement
  error = sen5x.startMeasurement();
  if (error) {
    Serial.print("Error trying to execute startMeasurement(): ");
    errorToString(error, errorMessage, 256);
    Serial.println(errorMessage);
  }

  //----------- BLE Set up-------------- //
  // 1. Service information to run once
  NimBLEDevice::init(deviceName);  // You need a device
  pServer = NimBLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // 2. Create Service
  NimBLEService* pService = pServer->createService(BLESERVICE_UUID);

  // 3. initialize Characteristics
  for (int i = 0; i < NumCharacteristics; i++) {
    pCharacteristicChars[i] = intializeBLECharacteristic(pCharacteristicChars[i], pService, characteristicUUIDs[i], UUIDLabels[i]);
  }

  pService->start();

  // 4. Get it going
  NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(BLESERVICE_UUID);
  pAdvertising->setScanResponse(false);
  NimBLEDevice::startAdvertising();
  Serial.println("Waiting for a client connection to notify...");


  // set transtiions
  idle->addTransition(&transitionidle2dataTaking, dataTaking);
  dataTaking->addTransition(&transitiondataTaking2idle, idle);  // transitions return the state they are heading to?
  // dataDumpMode->addTransition(&transition,S4);

  // enable UI components

  logEventButton = new Button();
  logEventButton->setup(
    logEventButtonPin, [] {
      logEventButton->handleInterrupt();
    },
    RISING);

  stateButton = new Button();
  stateButton->setup(
    stateButtonPin, [] {
      stateButton->handleInterrupt();
    },
    RISING);
}

// Transitions are evaluated by the state machine after the state logic has executed. If none of the transitions evaluate to true, then the machine stays in the current state.
GPSData GPS2Transmit;

void loop() {
  machine.run();
  // delay(1000);
}

void idleState() {
  Serial.printf("Idle State ! Nothing 2 c here :) \n");
  // Serial.printf("Just waiting until the state button is pressed to change:  %d \n", stateButton->getCount());
  StatusLED.ledSet(0, 0, timeDelayMS);

//   GPSData DummyGPSData = readAndStoreGPS();  // Hello GPS are you there
//   if (DummyGPSData.dataReceived == true) {   // Essential GPS Logic!
//     GPS2Transmit = DummyGPSData;
//   }

//   // waiting for left button to be pressed // (so this is transition state)
//   // light does not blink //
//   // probably run GPS in order to so that its ready to go for later!
}

bool transitionidle2dataTaking() {
  int buttonCountForPrint = stateButton->getCount();
  if (buttonCountForPrint != 0) {
    Serial.printf("Beeep Boop time to swap state! Idle -> Data Taking  \n");
    stateButton->resetCount();
    return true;
  }
  return false;
}

void dataTakingState() {
  // Serial.printf("Data time!  \n");
  StatusLED.ledSet(brightHigh, brightLow, timeDelayMS);
  //  if ( (millis() - GlobalTimer > sampleRateGps)) { // (millis() - GlobalTimer > sampleRateGps
  // if ((millis() - GlobalTimer > sampleRateGps)) {
  //   GlobalTimer = millis();
  //   std::vector<std::string> EncodedData = PullAndTranscribeData(GPS2Transmit);

  //   std::string Row_Data;

  //   for (int i = 0; i < NumCharacteristics; i++) {
  //     // Serial.print(i);
  //     // Serial.println(EncodedData[i].c_str());

  //     // Uncomment for BLE
  //     // pCharacteristicChars[i]->setValue(std::string(EncodedData[i]));
  //     // pCharacteristicChars[i]->notify();
  //     Row_Data += EncodedData[i].c_str();
  //     // Row_Data += ',';
  //     // Serial.println(Row_Data.c_str());
  //   }
  //   Serial.println(Row_Data.c_str());

  //   Row_Data += '\n';
  //   if (isMemoryCardAttached) {
  //     appendFile(SD, NewFilePath, Row_Data.c_str());  // Corrected the function name to appendFile and added missing ".c_str()"
  //   }
  //   //   if (deviceConnected && !oldDeviceConnected) {
  //   // oldDeviceConnected = deviceConnected;
  // }
}

bool transitiondataTaking2idle() {
  // Check if we really meant to transition
  int buttonCountForPrint = stateButton->getCount();
  if (buttonCountForPrint != 0) {

    Serial.printf("Beeep Boop time to swap state! Data Taking -> Idle \n");
    stateButton->resetCount();

    // eventCounter = PullLastEventIndex(SD, eventIndexPath);

    // sprintf(NewFilePath, "/Event%d.txt", eventCounter);  // Fixed format string
    // Serial.print("New data file created");
    // Serial.println(NewFilePath);
    // String Header = "TimeStamp, Latitude, Longitude, Altitude, PM25, RelativeHumidity, Temperature, vocIndex, noxIndex, PM10, ButtonPress";
    // writeFile(SD, NewFilePath, Header.c_str());  // Added missing ".c_str()" and semicolon
    // isEventStarted = true;
    // return true;
  }

  return false;
}

void dataDumpState() {
  Serial.printf("This actually should only run once to completion so creating a snappy little print statment is really not as important as writting something clear and robust, furthermore..   \n");
}

bool transitionidle2dataDumpState() {
}


// BLEService* pService = pServer->createService(BLESERVICE_UUID, NumCharacteristics, 0);

// NimBLEService* pService = pServer->createService(BLESERVICE_UUID, numServices, 0);

// // 2. Set Characteristics
// Serial.println(NumCharacteristics);
// for (int i = 0; i < NumCharacteristics; i++) {
//   Serial.println(characteristicUUIDs[i]);
//   pCharacteristicChars[i] = intializeBLECharacteristic(pCharacteristicChars[i], pService, characteristicUUIDs[i], UUIDLabels[i]);
// }

// // 3. Start the service
// pService->start();

// 4. Start advertising
// BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
// pAdvertising->addServiceUUID(BLESERVICE_UUID);
// pAdvertising->setScanResponse(false);
// pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
// BLEDevice::startAdvertising();
// Serial.println("Waiting a client connection to notify...");