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

void setup() {

  Serial.begin(115200);

  //----------- GPS Set up-------------- //
  GPSSerial.begin(9600);  // #fixme (should I be worried about this?)
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_5HZ);  // 1 Hz update rate

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
  // notify changed value
  GPSData DummyGPSData = readAndStoreGPS();  // Hello GPS are you still there
  
  if (DummyGPSData.dataReceived == true) {
    GPSData GPS2Transmit = DummyGPSData;
    Serial.println(GPS2Transmit.ShortTimeStamp);
  }

 if  ((deviceConnected) && (millis() - GlobalTimer > sampleRateGps)) {
    GlobalTimer = millis();
    std::string DummyTimestamp = "04:32:27 13/06/0023";
    pCharacteristicChars[0]->setValue(DummyTimestamp);
    pCharacteristicChars[0]->notify();
  }
  
  // Disconnecting
  if (!deviceConnected && oldDeviceConnected && (millis() - deviceRebootMillis > 500) ) {
    deviceRebootMillis = millis();
    pServer->startAdvertising();  // restart advertising
    Serial.println("start advertising");
    oldDeviceConnected = deviceConnected;
  }
  // Connecting
  if (deviceConnected && !oldDeviceConnected) {
    oldDeviceConnected = deviceConnected;
  }
}