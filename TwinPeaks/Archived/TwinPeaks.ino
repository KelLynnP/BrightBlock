#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#include <Adafruit_GPS.h>
#include "Adafruit_PM25AQI.h"
#include <Adafruit_BME680.h>
#include <Adafruit_ICM20948.h>
#include <Arduino.h>  // Sensirion I2C libraries (lol @ not using arduino?)
#include <SensirionI2CSen5x.h>
#include <Wire.h>

#include "FS.h"  // Memory Card files
#include "SD.h"
#include "SPI.h"

#include <ArduinoJson.h>
#include <unordered_map>
#include <SoftwareSerial.h>
#include <vector>
#include <string>

// ---------------------------------------------------------------//
// ------------------ User input Info ----------------------------//
// ---------------------------------------------------------------//
const char* deviceName = "BLEEP";
unsigned long previousMillis = 0;
const unsigned long sampleRate = 1000;
const int NumCharacteristics = 10;
const boolean isMemoryCardAttached = true;
const int memoryCardPin = 4;
int eventCounter = -1;
bool isEventStarted = false;
const char eventIndexPath[] = "/eventIndex.txt";
char NewFilePath[30];  // Increased the buffer size


// -------------------- Which Sensors! ------------------------------//

// #FIXME: Ideally this is a a config file which is updated at the start of each run!

const char* UUIDLabels[] = {
  "TimeStamp",
  "Latitude",
  "Longitude",
  "Altitude",
  "PM25",
  "RelativeHumidity",
  "Temperature",
  "AccelerationX",
  "AccelerationY",
  "AccelerationZ",
  // "Button Press"
};

const char* characteristicUUIDs[] = {
  "beb5483e-36e1-4688-b7f5-ea07361b26a8",
  "1c95d5e3-d8f7-413a-bf3d-7a2e5d7be87e",  //
  "d7d85823-5304-4eb3-9671-3e571fac07b9",
  "d2789cef-106f-4660-9e3f-584c12e2e3c7",
  "bf5a799d-26d0-410e-96b0-9ada1eb9f758",
  "c22b405e-2b7b-4632-831d-54523e169a01",
  "ffdda8ad-60a2-4184-baff-5c79a2eccb8c",
  "183b971a-79f5-4004-8182-31c88d910dca",  //
  "90b77f62-003d-454e-97fc-8f597b42048c",
  "86cef02b-8c15-457b-b480-52e6cc0bdd8c",  //
  // "755c7c73-b938-4a6e-a7be-2a3b8c3783d9", // Include for button presses later!
};

// ---------------------------------------------------------------//
// ------------------ GPS information ----------------------------//
// ---------------------------------------------------------------//

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

// ---------------------------------------------------------------//
// ------------------ BLE information ----------------------------//
// ---------------------------------------------------------------//

// Initialized Once // Not Characterisitic Specific
static BLEUUID BLESERVICE_UUID("4fafc201-1fb5-459e-8fcc-c5c9c331914b");
const int numServices = 100;  //#FIXME: not notifiying if failure of
bool deviceConnected = false;
bool oldDeviceConnected = false;
BLEServer* pServer = NULL;  // Pointer to the server
int value = 0;
unsigned long deviceRebootMillis = 0;

// Characteristic Specific
BLEDescriptor* pDescr;  // Pointer to Descriptor of Characteristic 1
BLE2902* pBLE2902;      // Pointer to BLE2902 of Characteristic 1
BLECharacteristic* pCharacteristicChars[NumCharacteristics] = { NULL };


class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
  };

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
  };
};

BLECharacteristic* intializeBLECharacteristic(BLECharacteristic* pCharacteristic, BLEService* pService, const char* characteristicUUID, const char* label) {
  // 2.a. Characteristics Here
  pCharacteristic = pService->createCharacteristic(
    characteristicUUID,
    BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_READ);
  // BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY);

  // 2.b.i Create a BLE Descriptor #FIXME: descriptors not working correctly, at least I cant see the label in NRF Connect
  pDescr = new BLEDescriptor((uint16_t)0x2901);  //The 0x2901 is the Bluetooth SIG-defined UUID for the "Characteristic User Description" descriptor. This descriptor is used to provide a human-readable description or name for the characteristic.
  Serial.print(characteristicUUID);
  pDescr->setValue(label);
  pCharacteristic->addDescriptor(pDescr);

  //2.b.ii Add the BLE2902 Descriptor because we are using "PROPERTY_NOTIFY"
  pBLE2902 = new BLE2902();
  pBLE2902->setNotifications(true);
  pCharacteristic->addDescriptor(pBLE2902);
  return pCharacteristic;
}

// ---------------------------------------------------------------//
// -------- Sensor Declarations + Helper Functions ---------------//
// ---------------------------------------------------------------//

// Sensirion Data Rhings The used commands use up to 48 bytes. On some Arduino's the default buffer space is not large enough
#define MAXBUF_REQUIREMENT 48

#if (defined(I2C_BUFFER_LENGTH) && (I2C_BUFFER_LENGTH >= MAXBUF_REQUIREMENT)) || (defined(BUFFER_LENGTH) && BUFFER_LENGTH >= MAXBUF_REQUIREMENT)
#define USE_PRODUCT_INFO
#endif

SensirionI2CSen5x sen5x;

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
  char Data[15];
  // Serial.println(RawData);
  sprintf(Data, ",%f", RawData);
  std::string str = std::string(TimeSnip) + Data;
  Serial.println(str.c_str());
  return str;
}

std::vector<std::string> PullAndTranscribeData(const GPSData& GPSData2Transmit) {
  std::vector<std::string> sensorDataVector(NumCharacteristics);
  // ICMData ICM = getICMData();
  char errorMessage[256];
  uint16_t error;

  // Read Measurement
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
  sensorDataVector[4] = FormatAndAppendTimestamp(massConcentrationPm2p5, GPSData2Transmit.ShortTimeStamp);

  //relative Humidity
  sensorDataVector[5] = FormatAndAppendTimestamp(ambientHumidity, GPSData2Transmit.ShortTimeStamp);
  sensorDataVector[6] = FormatAndAppendTimestamp(ambientTemperature, GPSData2Transmit.ShortTimeStamp);

  // Filling in with more climate data for now
  sensorDataVector[7] = FormatAndAppendTimestamp(vocIndex, GPSData2Transmit.ShortTimeStamp);
  sensorDataVector[8] = FormatAndAppendTimestamp(noxIndex, GPSData2Transmit.ShortTimeStamp);
  sensorDataVector[9] = FormatAndAppendTimestamp(massConcentrationPm10p0, GPSData2Transmit.ShortTimeStamp);
  // sensorDataVector[10] = FormatAndAppendTimestamp(massConcentrationPm10p0, GPSData2Transmit.ShortTimeStamp);

  // // Accerleration Data
  // sensorDataVector[7] = FormatAndAppendTimestamp(ICM.AccelX, GPSData2Transmit.ShortTimeStamp);
  // sensorDataVector[8] = FormatAndAppendTimestamp(ICM.AccelY, GPSData2Transmit.ShortTimeStamp);
  // sensorDataVector[9] = FormatAndAppendTimestamp(ICM.AccelZ, GPSData2Transmit.ShortTimeStamp);

  return sensorDataVector;
}


// ---------------------------------------------------------------//
// ---------------------- Button Functions -----------------------//
// ---------------------------------------------------------------//

const int buttonPin = 36;  // the number of the pushbutton pin
int buttonStateCount = 0;


// int countButtonHigh

// ---------------------------------------------------------------//
// ------------------ Memory Card Stuff --------------------------//
// ---------------------------------------------------------------//

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
#ifdef USE_PRODUCT_INFO
  printSerialNumber();
  printModuleVersions();
#endif

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
  BLEDevice::init(deviceName);  // You need a device
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  // BLEService* pService = pServer->createService(BLESERVICE_UUID, NumCharacteristics, 0);

  BLEService* pService = pServer->createService(BLESERVICE_UUID, numServices, 0);

  // 2. Set Characteristics
  Serial.println(NumCharacteristics);
  for (int i = 0; i < NumCharacteristics; i++) {
    Serial.println(characteristicUUIDs[i]);
    pCharacteristicChars[i] = intializeBLECharacteristic(pCharacteristicChars[i], pService, characteristicUUIDs[i], UUIDLabels[i]);
  }

  // 3. Start the service
  pService->start();

  // 4. Start advertising
  BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(BLESERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
  BLEDevice::startAdvertising();
  Serial.println("Waiting a client connection to notify...");
}

GPSData GPS2Transmit;

void loop() {

  GPSData DummyGPSData = readAndStoreGPS();  // Hello GPS are you there

  if (DummyGPSData.dataReceived == true) {  // Essential GPS Logic!
    GPS2Transmit = DummyGPSData;
  }

  //  if ( (millis() - GlobalTimer > sampleRateGps)) { // (millis() - GlobalTimer > sampleRateGps
  if ((deviceConnected) && (millis() - GlobalTimer > sampleRateGps)) {
    GlobalTimer = millis();
    std::vector<std::string> EncodedData = PullAndTranscribeData(GPS2Transmit);

    std::string Row_Data;
    for (int i = 0; i < NumCharacteristics; i++) {  //
      // Serial.print(i);
      // Serial.println(EncodedData[i].c_str());
      pCharacteristicChars[i]->setValue(std::string(EncodedData[i]));
      pCharacteristicChars[i]->notify();
      Row_Data += EncodedData[i].c_str();
      // Row_Data += ',';
      // Serial.println(Row_Data.c_str());
    }
    Serial.println(Row_Data.c_str());

    Row_Data += '\n';
    if (isMemoryCardAttached) {
      appendFile(SD, NewFilePath, Row_Data.c_str());  // Corrected the function name to appendFile and added missing ".c_str()"
    }
  }


  // --------- BLE LOGIC FOR CONNECTION AND DISCONNECTION --------//
  // Disconnection
  if (!deviceConnected && oldDeviceConnected && (millis() - deviceRebootMillis > 500)) {
    deviceRebootMillis = millis();
    pServer->startAdvertising();  // restart advertising
    Serial.println("start advertising");
    oldDeviceConnected = deviceConnected;
    isEventStarted = false;
  }

  // Connection
  if (deviceConnected && !oldDeviceConnected) {
    oldDeviceConnected = deviceConnected;

    // pull last Event Started
    if (isEventStarted == false) {
      eventCounter = PullLastEventIndex(SD, eventIndexPath);
      sprintf(NewFilePath, "/Event%d.txt", eventCounter);  // Fixed format string
      Serial.print("New data file created");
      Serial.println(NewFilePath);
      String Header = "TimeStamp, Latitude, Longitude, Altitude, PM25, RelativeHumidity, Temperature, AccelerationX, AccelerationY, AccelerationZ";
      writeFile(SD, NewFilePath, Header.c_str());  // Added missing ".c_str()" and semicolon
      isEventStarted = true;
    }
  }
}



// Adafruit_ICM20948 icm;  // Initialize the ICM209448 sensor object
// unsigned long PlantMillis = 0;
// const unsigned long PlantSample = 500;

// struct ICMData {
//   float AccelX;  // acceleration
//   float AccelY;
//   float AccelZ;
// };

// ICMData getICMData();

// ICMData getICMData() {
//   ICMData ICM;
//   sensors_event_t accel;
//   sensors_event_t gyro;
//   sensors_event_t mag;
//   sensors_event_t temp;
//   icm.getEvent(&accel, &gyro, &temp, &mag);
//   // GTemp = temp.temperature; // not pulling a second temperature value but we could

//   ICM.AccelX = accel.acceleration.x;  // acceleration
//   ICM.AccelY = accel.acceleration.y;
//   ICM.AccelZ = accel.acceleration.z;
//   return ICM;
// }


// ICM Begin code
// if (!icm.begin_I2C()) {  // if (!icm.begin_SPI(ICM_CS)) { // if (!icm.begin_SPI(ICM_CS, ICM_SCK, ICM_MISO, ICM_MOSI)) {

//   Serial.println("Failed to find ICM20948 chip");
//   while (1) {
//     delay(10);
//     break;
//   }
// } else {
//   Serial.println("ICM20948 Found!");
//   // icm.setAccelRange(ICM20948_ACCEL_RANGE_16_G);
//   Serial.print("Accelerometer range set to: ");
//   switch (icm.getAccelRange()) {
//     case ICM20948_ACCEL_RANGE_2_G:
//       Serial.println("+-2G");
//       break;
//     case ICM20948_ACCEL_RANGE_4_G:
//       Serial.println("+-4G");
//       break;
//     case ICM20948_ACCEL_RANGE_8_G:
//       Serial.println("+-8G");
//       break;
//     case ICM20948_ACCEL_RANGE_16_G:
//       Serial.println("+-16G");
//       break;
//   }
//   // Serial.println("OK");
//   // icm.setGyroRange(ICM20948_GYRO_RANGE_2000_DPS);
//   Serial.print("Gyro range set to: ");
//   switch (icm.getGyroRange()) {
//     case ICM20948_GYRO_RANGE_250_DPS:
//       Serial.println("250 degrees/s");
//       break;
//     case ICM20948_GYRO_RANGE_500_DPS:
//       Serial.println("500 degrees/s");
//       break;
//     case ICM20948_GYRO_RANGE_1000_DPS:
//       Serial.println("1000 degrees/s");
//       break;
//     case ICM20948_GYRO_RANGE_2000_DPS:
//       Serial.println("2000 degrees/s");
//       break;
//   }

//   //  icm.setAccelRateDivisor(4095);
//   uint16_t accel_divisor = icm.getAccelRateDivisor();
//   float accel_rate = 1125 / (1.0 + accel_divisor);

//   Serial.print("Accelerometer data rate divisor set to: ");
//   Serial.println(accel_divisor);
//   Serial.print("Accelerometer data rate (Hz) is approximately: ");
//   Serial.println(accel_rate);

//   //  icm.setGyroRateDivisor(255);
//   uint8_t gyro_divisor = icm.getGyroRateDivisor();
//   float gyro_rate = 1100 / (1.0 + gyro_divisor);

//   Serial.print("Gyro data rate divisor set to: ");
//   Serial.println(gyro_divisor);
//   Serial.print("Gyro data rate (Hz) is approximately: ");
//   Serial.println(gyro_rate);

//   // icm.setMagDataRate(AK09916_MAG_DATARATE_10_HZ);
//   Serial.print("Magnetometer data rate set to: ");
//   switch (icm.getMagDataRate()) {
//     case AK09916_MAG_DATARATE_SHUTDOWN:
//       Serial.println("Shutdown");
//       break;
//     case AK09916_MAG_DATARATE_SINGLE:
//       Serial.println("Single/One shot");
//       break;
//     case AK09916_MAG_DATARATE_10_HZ:
//       Serial.println("10 Hz");
//       break;
//     case AK09916_MAG_DATARATE_20_HZ:
//       Serial.println("20 Hz");
//       break;
//     case AK09916_MAG_DATARATE_50_HZ:
//       Serial.println("50 Hz");
//       break;
//     case AK09916_MAG_DATARATE_100_HZ:
//       Serial.println("100 Hz");
//       break;
//   }
//   Serial.println();
// }
