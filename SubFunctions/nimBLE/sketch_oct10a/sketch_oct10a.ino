#include <NimBLEDevice.h>
#include <unordered_map>

static NimBLEUUID BLESERVICE_UUID("4fafc201-1fb5-459e-8fcc-c5c9c331914b");
bool deviceConnected = false;
bool oldDeviceConnected = false;
static NimBLEServer* pServer;

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
  "Altitude"
};

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
    NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::READ
  );

  pCharacteristic->createDescriptor("2901", NIMBLE_PROPERTY::READ)->setValue(label);

  // pCharacteristic->createDescriptor(NimBLEUUID((uint16_t)0x2902), NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::NOTIFY);

  return pCharacteristic;
}

NimBLECharacteristic* pCharacteristicChars[4] = { NULL };

void setup() {
  Serial.begin(115200);

  NimBLEDevice::init("Bleep");
  pServer = NimBLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  NimBLEService* pService = pServer->createService(BLESERVICE_UUID);

  for (int i = 0; i < 4; i++) {
    pCharacteristicChars[i] = intializeBLECharacteristic(pCharacteristicChars[i], pService, characteristicUUIDs[i], UUIDLabels[i]);
  }

  pService->start();

  NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(BLESERVICE_UUID);
  pAdvertising->setScanResponse(false);
  NimBLEDevice::startAdvertising();
  Serial.println("Waiting for a client connection to notify...");
}

void loop() {
  std::string DummyTimestamp = "04:32:27 13/06/0023";

  for(int i=0; i<4; i++) {
    pCharacteristicChars[i]->setValue(DummyTimestamp);
    pCharacteristicChars[i]->notify();
    delay(500);
  }

  if (!deviceConnected && oldDeviceConnected) {
    delay(500);
    NimBLEDevice::startAdvertising();
    Serial.println("start advertising");
    oldDeviceConnected = deviceConnected;
  }

  if (deviceConnected && !oldDeviceConnected) {
    Serial.println("Device Connected");
    oldDeviceConnected = deviceConnected;
  }
}
