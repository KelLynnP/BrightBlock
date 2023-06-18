#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <Adafruit_GPS.h>
#include <ArduinoJson.h>
#include <unordered_map>
#include <string>
#include <SoftwareSerial.h>


// ---------------------------------------------------------------//
// ------------------ User input Info ----------------------------//
// ---------------------------------------------------------------//
const char* deviceName = "BLEEP";
unsigned long previousMillis = 0;
const unsigned long sampleRate = 1000;

// ---------------------------------------------------------------//
// ------------------ BLE information ----------------------------//
// ---------------------------------------------------------------//

// #FIXME - add more pointers Initialize all pointers
BLEServer* pServer = NULL;                    // Pointer to the server
BLECharacteristic* pCharacteristic_1 = NULL;  // Pointer to Characteristic 1
BLECharacteristic* pCharacteristic_2 = NULL;  // Pointer to Characteristic 2
BLEDescriptor* pDescr_1;                      // Pointer to Descriptor of Characteristic 1
BLE2902* pBLE2902_1;                          // Pointer to BLE2902 of Characteristic 1
BLE2902* pBLE2902_2;                          // Pointer to BLE2902 of Characteristic 2

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/
// UUIDs used in this example:
static BLEUUID BLESERVICE_UUID("4fafc201-1fb5-459e-8fcc-c5c9c331914b");
// #define CHARACTERISTIC_UUID_1 "beb5483e-36e1-4688-b7f5-ea07361b26a8"
// #define CHARACTERISTIC_UUID_2 "1c95d5e3-d8f7-413a-bf3d-7a2e5d7be87e"

const char* characteristicUUIDs[] = {
  "beb5483e-36e1-4688-b7f5-ea07361b26a8",
  "1c95d5e3-d8f7-413a-bf3d-7a2e5d7be87e",
};

// class SensorObject {
//   public:

//   SensorObject(char* UUID, char* label, int SampleRate){
//     this->UUID = UUID;
//     this->label = label;
//     this->SampleRate = SampleRate;
//   }

//   // Member Attributes
//   char UUID;
//   char label;
//   int SampleRate;
// };

const char* dataLabelList[] = {
  "TimeStamp",
  "Latitude",
  "Longitude",
};

const char* sampleRateList[] = {
  "5000",
  "5000",
  "5000",
};

struct sensorDataItem {
  std::string UUID;
  std::string label;
  std::string sampleRate;
};


// Some variables to keep track on device connected
const int numCharacteristics = sizeof(characteristicUUIDs) / sizeof(characteristicUUIDs[0]);
bool deviceConnected = false;
bool oldDeviceConnected = false;
float value = 0.0f;
float value_2 = 0.0f;
int packet_count = 0;


// Callback function that is called whenever a client is connected or disconnected
class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
  };

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
  }
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
bool FirstTime = false;          // ugh so annoying for pre-allocating

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
      // Serial.print(c);  // if a sentence is received, we can check the checksum, parse it...
    }
  }

  if (GPS.newNMEAreceived()) {  // a tricky thing here is if we print the NMEA sentence, or data // we end up not listening and catching other sentences! // so be very wary if using OUTPUT_ALLDATA and trying to print out data     // Serial.print(GPS.lastNMEA()); // this also sets the newNMEAreceived() flag to false
    if (!GPS.parse(GPS.lastNMEA())) // this also sets the newNMEAreceived() flag to fals
      return TempGPS;
    // Serial.print(GPS.seconds);
    // Serial.println("-> seconds! ");
  }

  if ((millis() - GpsTimer > sampleRateGps) && (!GPS.parse(GPS.lastNMEA())) && (GPS.fix != 0)) {
    GpsTimer = millis();  // reset the timer
    
    // Serial.print("Fix: "); Serial.print((int)GPS.fix);
    // Serial.print(" quality: "); Serial.println((int)GPS.fixquality);
    sprintf(TempGPS.FullTimeStamp, "%02d:%02d:%02d %02d/%02d/%04d", GPS.hour, GPS.minute, GPS.seconds, GPS.day, GPS.month, GPS.year);
    sprintf(TempGPS.ShortTimeStamp, "%02d%02d%02d", GPS.hour, GPS.minute, GPS.seconds);
    TempGPS.latitude = GPS.latitude;
    TempGPS.longitude = GPS.longitude;
    TempGPS.altitude = GPS.altitude;
    TempGPS.dataReceived = true;
    // ~~~~~~~~~~~~~~~~~~~ Run these lines to debug ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
    // Serial.print(TempGPS.latitude); Serial.println(TempGPS.longitude);
    // Serial.print(GPS.seconds);
    // Serial.println("-> seconds! ");
    // Serial.print(GPS.hour);Serial.println("-> hour! ");
    // Serial.print(GPS.year);Serial.println("-> year! ");
    // Serial.println(TempGPS.TimeStamp);
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


void setup() {

  Serial.begin(115200);

  GPSSerial.begin(9600);  // #fixme (should I be worried about this?)
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_5HZ);  // 1 Hz update rate


  BLEDevice::init(deviceName);

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService* pService = pServer->createService(BLESERVICE_UUID, 30, 0);

  // Create a BLE Characteristic
  pCharacteristic_1 = pService->createCharacteristic(
    characteristicUUIDs[0],
    BLECharacteristic::PROPERTY_NOTIFY);

  pCharacteristic_2 = pService->createCharacteristic(
    characteristicUUIDs[1],
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY);

  // Create a BLE Descriptor
  pDescr_1 = new BLEDescriptor((uint16_t)0x2901);  //The 0x2901 is the Bluetooth SIG-defined UUID for the "Characteristic User Description" descriptor. This descriptor is used to provide a human-readable description or name for the characteristic.
  pDescr_1->setValue("Test Value");
  pCharacteristic_1->addDescriptor(pDescr_1);

  // Add the BLE2902 Descriptor because we are using "PROPERTY_NOTIFY"
  pBLE2902_1 = new BLE2902();
  pBLE2902_1->setNotifications(true);
  pCharacteristic_1->addDescriptor(pBLE2902_1);

  pBLE2902_2 = new BLE2902();
  pBLE2902_2->setNotifications(true);
  pCharacteristic_2->addDescriptor(pBLE2902_2);

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(BLESERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
  BLEDevice::startAdvertising();
  Serial.println("Waiting a client connection to notify...");
}

GPSData GPS2Transmit;
void loop() {

  // notify changed value
  GPSData DummyGPSData = readAndStoreGPS();  // Hello GPS are you still there
  if (DummyGPSData.dataReceived == true) {
    GPSData GPS2Transmit = DummyGPSData;
    Serial.println(GPS2Transmit.ShortTimeStamp);
  }
 if ( (millis() - GlobalTimer > sampleRateGps)) {
  // if ((deviceConnected) && (millis() - GlobalTimer > sampleRateGps)) {
    GlobalTimer = millis();
    std::string DummyTimestamp = "04:32:27 13/06/0023";
    // Serial.println(GPS2Transmit.ShortTimeStamp);
    // Serial.print(GPS.seconds);
    // Serial.println("-> seconds! ");


    pCharacteristic_1->setValue(DummyTimestamp);
    // pCharacteristic_1->setValue(std::string myString(GPS2Transmit.ShortTimeStamp))
    pCharacteristic_1->notify();
    value++;
    // Serial.println(value_2);
    value_2--;
    std::string value_2_string = std::to_string(value_2);

    // // Here the value is written to the Client using setValue();
    // String txValue = "String with random value from Server: " + String(random(1000));
    pCharacteristic_2->setValue(value_2_string);
    pCharacteristic_2->notify();
  }
  // The code below keeps the connection status uptodate:
  // Disconnecting
  if (!deviceConnected && oldDeviceConnected) {
    //  delay(500); // #Fix me and remove delay
    pServer->startAdvertising();  // restart advertising
    Serial.println("start advertising");
    oldDeviceConnected = deviceConnected;
  }
  // Connecting
  if (deviceConnected && !oldDeviceConnected) {
    // do stuff here on connecting
    oldDeviceConnected = deviceConnected;
  }
}


// GRAVEYARD



// Serial.println("Characteristic 2 (setValue): " + txValue);

// pCharacteristic_2 is a std::string (NOT a String). In the code below we read the current value
// write this to the Serial interface and send a different value back to the Client
// Here the current value is read using getValue()
// std::string rxValue = pCharacteristic_2->getValue();
// Serial.print("Characteristic 2 (getValue): ");
// Serial.println(rxValue.c_str());

// FIXME:  remove delay, GPS code wont work like this
// delay(1000);
