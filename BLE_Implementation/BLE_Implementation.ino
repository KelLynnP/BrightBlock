#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <Adafruit_GPS.h>
#include <ArduinoJson.h>
#include <unordered_map>
#include <SoftwareSerial.h>
#include <vector>
#include <string>

#include "Adafruit_PM25AQI.h"
#include <Adafruit_BME680.h>
#include <Adafruit_ICM20948.h>


// ---------------------------------------------------------------//
// ------------------ User input Info ----------------------------//
// ---------------------------------------------------------------//
const char* deviceName = "BLEEP";
unsigned long previousMillis = 0;
const unsigned long sampleRate = 1000;

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
  bool dataReceived = false; // set true if set correctly
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

  if (GPS.newNMEAreceived()) {  // a tricky thing here is if we print the NMEA sentence, or data // we end up not listening and catching other sentences! // so be very wary if using OUTPUT_ALLDATA and trying to print out data     // Serial.print(GPS.lastNMEA()); // this also sets the newNMEAreceived() flag to false
    if (!GPS.parse(GPS.lastNMEA())) // this also sets the newNMEAreceived() flag to fals
      return TempGPS;
  }

  if ((millis() - GpsTimer > sampleRateGps) && (!GPS.parse(GPS.lastNMEA())) && (GPS.fix != 0)) {
    GpsTimer = millis();  // reset the timer
    sprintf(TempGPS.FullTimeStamp, "%02d:%02d:%02d %02d/%02d/%04d", GPS.hour, GPS.minute, GPS.seconds, GPS.day, GPS.month, GPS.year);
    sprintf(TempGPS.ShortTimeStamp, "%02d%02d%02d", GPS.hour, GPS.minute, GPS.seconds);
    TempGPS.latitude =  GPS.latitudeDegrees;
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
const int numServices = 30;
bool deviceConnected = false;
bool oldDeviceConnected = false;
BLEServer* pServer = NULL;                    // Pointer to the server
int value = 0;
unsigned long deviceRebootMillis = 0;

// Characteristic Specific
BLEDescriptor* pDescr;                      // Pointer to Descriptor of Characteristic 1
BLE2902* pBLE2902;                          // Pointer to BLE2902 of Characteristic 1
BLECharacteristic* pCharacteristicChars[9] = {NULL};

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
  "86cef02b-8c15-457b-b480-52e6cc0bdd8c"
};

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
  "AccelerationZ"
};

class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
  };

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
  };
};

BLECharacteristic* intializeBLECharacteristic(BLECharacteristic* pCharacteristic, BLEService* pService, const char*  characteristicUUID,const char*  label){
  // 2.a. Characteristics Here 
  pCharacteristic = pService->createCharacteristic(
  characteristicUUID,
  BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_READ);
  // BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY);
  
  // 2.b.i Create a BLE Descriptor #FIXME: descriptors not working correctly, at least I cant see the label in NRF Connect
  pDescr = new BLEDescriptor((uint16_t)0x2901);  //The 0x2901 is the Bluetooth SIG-defined UUID for the "Characteristic User Description" descriptor. This descriptor is used to provide a human-readable description or name for the characteristic.
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

Adafruit_PM25AQI aqi = Adafruit_PM25AQI();
PM25_AQI_Data PM25Data;
Adafruit_BME680 bme; // Initialize the BME688 sensor object
Adafruit_ICM20948 icm; // Initialize the ICM209448 sensor object
unsigned long PlantMillis = 0;
const unsigned long PlantSample = 500;

struct ICMData{
  float AccelX;  // acceleration
  float AccelY; 
  float AccelZ; 
};

ICMData getICMData();

ICMData getICMData(){
  ICMData ICM;
  sensors_event_t accel;
  sensors_event_t gyro;
  sensors_event_t mag;
  sensors_event_t temp;
  icm.getEvent(&accel, &gyro, &temp, &mag);
  // GTemp = temp.temperature; // not pulling a second temperature value but we could

  ICM.AccelX = accel.acceleration.x; // acceleration
  ICM.AccelY = accel.acceleration.y;
  ICM.AccelZ = accel.acceleration.z;
  return ICM;
}

std::vector<std::string> PullAndTranscribeData(const GPSData& GPSData2Transmit) {
  std::vector<std::string> sensorDataVector(9);
  ICMData ICM = getICMData(); 

  // GPS Data All pulled seperatly 
  sensorDataVector[0] = GPSData2Transmit.FullTimeStamp;

  sensorDataVector[1] = FormatAndAppendTimestamp(GPSData2Transmit.latitude, GPSData2Transmit.ShortTimeStamp);
  sensorDataVector[2] = FormatAndAppendTimestamp(GPSData2Transmit.longitude, GPSData2Transmit.ShortTimeStamp);
  sensorDataVector[3] = FormatAndAppendTimestamp(GPSData2Transmit.altitude, GPSData2Transmit.ShortTimeStamp);
  sensorDataVector[4] = FormatAndAppendTimestamp(float(PM25Data.pm25_env), GPSData2Transmit.ShortTimeStamp);
  
  //relative Humidity 
  sensorDataVector[5] = FormatAndAppendTimestamp(bme.readHumidity(), GPSData2Transmit.ShortTimeStamp);
  sensorDataVector[6] = FormatAndAppendTimestamp(bme.readTemperature(), GPSData2Transmit.ShortTimeStamp);

  // Accerleration Data
  // sensorDataVector[7] = FormatAndAppendTimestamp(ICM.AccelX, GPSData2Transmit.ShortTimeStamp);
  // sensorDataVector[8] = FormatAndAppendTimestamp(ICM.AccelX, GPSData2Transmit.ShortTimeStamp);
  // sensorDataVector[9] = FormatAndAppendTimestamp(ICM.AccelZ, GPSData2Transmit.ShortTimeStamp);

  return sensorDataVector;
}

std::string FormatAndAppendTimestamp(float RawData, const char* TimeSnip) {
  char Data[15];
  // Serial.println(RawData);
  sprintf(Data, ",%f", RawData);
  std::string str = std::string(TimeSnip) + Data;
  Serial.println(str.c_str());
  return str;
}

// ---------------------------------------------------------------//
// ---------------------- Button Functions -----------------------//
// ---------------------------------------------------------------//

const int buttonPin = 25;  // the number of the pushbutton pin
int buttonState = 0;

void setup() {

  Serial.begin(115200);

  //----------- GPS Set up-------------- //
  GPSSerial.begin(9600);  // #fixme (should I be worried about this?)
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_5HZ);  // 1 Hz update rate

  //---------- PM 2.5 Set Up ------------ //
    // Wait one second for sensor to boot up!
  delay(1000);
  if (! aqi.begin_I2C()) {      // connect to the sensor over I2C
    Serial.println("Could not find PM 2.5 sensor!");
    while (1) delay(10);
  }
  Serial.println("PM25 found!");

  //---------- BME 688 Begin -----------//
    if (!bme.begin()) {
    Serial.println("Could not find BME688 sensor!");
  }
  else{    // ~if~ we find Set up oversampling and filter initialization
    bme.setTemperatureOversampling(BME680_OS_8X);
    bme.setHumidityOversampling(BME680_OS_2X);
    bme.setPressureOversampling(BME680_OS_4X);
    bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
    bme.setGasHeater(320, 150); // 320*C for 150 ms
  }

  //---------- ICM  Begin -----------//
  if (!icm.begin_I2C()) { // if (!icm.begin_SPI(ICM_CS)) { // if (!icm.begin_SPI(ICM_CS, ICM_SCK, ICM_MISO, ICM_MOSI)) {

    Serial.println("Failed to find ICM20948 chip");
    while (1) {
      delay(10);
      break;
    }
  }
  else{
  Serial.println("ICM20948 Found!");
  // icm.setAccelRange(ICM20948_ACCEL_RANGE_16_G);
  Serial.print("Accelerometer range set to: ");
  switch (icm.getAccelRange()) {
  case ICM20948_ACCEL_RANGE_2_G:
    Serial.println("+-2G");
    break;
  case ICM20948_ACCEL_RANGE_4_G:
    Serial.println("+-4G");
    break;
  case ICM20948_ACCEL_RANGE_8_G:
    Serial.println("+-8G");
    break;
  case ICM20948_ACCEL_RANGE_16_G:
    Serial.println("+-16G");
    break;
  }
  // Serial.println("OK");
  // icm.setGyroRange(ICM20948_GYRO_RANGE_2000_DPS);
  Serial.print("Gyro range set to: ");
  switch (icm.getGyroRange()) {
  case ICM20948_GYRO_RANGE_250_DPS:
    Serial.println("250 degrees/s");
    break;
  case ICM20948_GYRO_RANGE_500_DPS:
    Serial.println("500 degrees/s");
    break;
  case ICM20948_GYRO_RANGE_1000_DPS:
    Serial.println("1000 degrees/s");
    break;
  case ICM20948_GYRO_RANGE_2000_DPS:
    Serial.println("2000 degrees/s");
    break;
  }

  //  icm.setAccelRateDivisor(4095);
  uint16_t accel_divisor = icm.getAccelRateDivisor();
  float accel_rate = 1125 / (1.0 + accel_divisor);

  Serial.print("Accelerometer data rate divisor set to: ");
  Serial.println(accel_divisor);
  Serial.print("Accelerometer data rate (Hz) is approximately: ");
  Serial.println(accel_rate);

  //  icm.setGyroRateDivisor(255);
  uint8_t gyro_divisor = icm.getGyroRateDivisor();
  float gyro_rate = 1100 / (1.0 + gyro_divisor);

  Serial.print("Gyro data rate divisor set to: ");
  Serial.println(gyro_divisor);
  Serial.print("Gyro data rate (Hz) is approximately: ");
  Serial.println(gyro_rate);

  // icm.setMagDataRate(AK09916_MAG_DATARATE_10_HZ);
  Serial.print("Magnetometer data rate set to: ");
  switch (icm.getMagDataRate()) {
  case AK09916_MAG_DATARATE_SHUTDOWN:
    Serial.println("Shutdown");
    break;
  case AK09916_MAG_DATARATE_SINGLE:
    Serial.println("Single/One shot");
    break;
  case AK09916_MAG_DATARATE_10_HZ:
    Serial.println("10 Hz");
    break;
  case AK09916_MAG_DATARATE_20_HZ:
    Serial.println("20 Hz");
    break;
  case AK09916_MAG_DATARATE_50_HZ:
    Serial.println("50 Hz");
    break;
  case AK09916_MAG_DATARATE_100_HZ:
    Serial.println("100 Hz");
    break;
  }
  Serial.println();
  }


  //----------- BLE Set up-------------- //
  // 1. Service information to run once   
  BLEDevice::init(deviceName);     // You need a device 
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService* pService = pServer->createService(BLESERVICE_UUID, 30, 0);
  
  // 2. Set Characteristics 
  const int NumCharacteristics = sizeof(characteristicUUIDs) / sizeof(characteristicUUIDs[0]);
  for (int i = 0; i < NumCharacteristics; i ++){
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

    for( int i = 0 ; i < 1 ; i ++){ //NumCharacteristics
    Serial.println(EncodedData[i].c_str());
    pCharacteristicChars[i]->setValue(std::string(EncodedData[i]));
    pCharacteristicChars[i]->notify();
    }

  }


  // --------- BLE LOGIC FOR CONNECTION AND DISCONNECTION --------// 
  // Disconnection 
  if (!deviceConnected && oldDeviceConnected && (millis() - deviceRebootMillis > 500) ) {
    deviceRebootMillis = millis();
    pServer->startAdvertising();  // restart advertising
    Serial.println("start advertising");
    oldDeviceConnected = deviceConnected;
  }
  // Connection 
  if (deviceConnected && !oldDeviceConnected) {
    oldDeviceConnected = deviceConnected;
  }
}



