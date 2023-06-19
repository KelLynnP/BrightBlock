#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <unordered_map>
#include <string>

// Initialized Once // Not Characterisitic Specific  
static BLEUUID BLESERVICE_UUID("4fafc201-1fb5-459e-8fcc-c5c9c331914b");
const int numServices = 30;
bool deviceConnected = false;
bool oldDeviceConnected = false;
BLEServer* pServer = NULL;                    // Pointer to the server
int value = 0;

// Characteristic Specific
BLEDescriptor* pDescr;                      // Pointer to Descriptor of Characteristic 1
BLE2902* pBLE2902;                          // Pointer to BLE2902 of Characteristic 1
BLECharacteristic* pCharacteristicChars[9] = {NULL};

const char* characteristicUUIDs[] = {
  "beb5483e-36e1-4688-b7f5-ea07361b26a8",
  "1c95d5e3-d8f7-413a-bf3d-7a2e5d7be87e",
  "d7d85823-5304-4eb3-9671-3e571fac07b9",
  "d2789cef-106f-4660-9e3f-584c12e2e3c7",
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


// Let it run!!!!
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  // 1. Service information to run once 
  BLEDevice::init("Bleep");     // You need a device 
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService* pService = pServer->createService(BLESERVICE_UUID, 30, 0);
  
  // 2. Set Characteristics 
  for (int i = 0; i < 4; i ++){
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

void loop() {
  // 5. Send values
  std::string DummyTimestamp = "04:32:27 13/06/0023";
  pCharacteristicChars[0]->setValue(DummyTimestamp);
  pCharacteristicChars[0]->notify();

  pCharacteristicChars[1]->setValue(DummyTimestamp);
  pCharacteristicChars[1]->notify();

  pCharacteristicChars[2]->setValue(DummyTimestamp);
  pCharacteristicChars[2]->notify();

  delay(500);

  if (!deviceConnected && oldDeviceConnected) {
    delay(500); // #Fix me and remove delay
    pServer->startAdvertising();  // restart advertising
    Serial.println("start advertising");
    oldDeviceConnected = deviceConnected;
  }
  // Connecting
  if (deviceConnected && !oldDeviceConnected) {     // do stuff here on connecting
    Serial.println("Device Connected");
    oldDeviceConnected = deviceConnected;
  }
}