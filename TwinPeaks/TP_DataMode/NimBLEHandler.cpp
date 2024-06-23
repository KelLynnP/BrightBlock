// #include "NimBLEHandler.h"

// // pre Allocates Characteristic array 
// NimBLECharacteristic* NimBLEHandler::pCharacteristicChars[NimBLEHandler::NumCharacteristics] = { NULL };

// static NimBLEUUID BLESERVICE_UUID("4fafc201-1fb5-459e-8fcc-c5c9c331914b");

// const char* UUIDLabels[] = {
//   "TimeStamp",         // From GPS
//   "Latitude",          // From GPS
//   "Longitude",         // From GPS
//   "Altitude",          // From GPS
//   "PM25",              // From SEN55
//   "RelativeHumidity",  // From SEN55
//   "Temperature",       // From SEN55
//   "vocIndex",          // From SEN55
//   "noxIndex",          // From SEN55
//   "PM10",              // From SEN55
//   "ButtonPress"        // From Input button
// };

// const char* characteristicUUIDs[] = {
//   "beb5483e-36e1-4688-b7f5-ea07361b26a8",
//   "1c95d5e3-d8f7-413a-bf3d-7a2e5d7be87e",
//   "d7d85823-5304-4eb3-9671-3e571fac07b9",
//   "d2789cef-106f-4660-9e3f-584c12e2e3c7",
//   "bf5a799d-26d0-410e-96b0-9ada1eb9f758",
//   "c22b405e-2b7b-4632-831d-54523e169a01",
//   "ffdda8ad-60a2-4184-baff-5c79a2eccb8c",
//   "183b971a-79f5-4004-8182-31c88d910dca",
//   "90b77f62-003d-454e-97fc-8f597b42048c",
//   "86cef02b-8c15-457b-b480-52e6cc0bdd8c",
//   "755c7c73-b938-4a6e-a7be-2a3b8c3783d9",
// };

// // connection logic 
// bool NimBLEHandler::deviceConnected = false;
// bool NimBLEHandler::oldDeviceConnected = false;
// NimBLEServer* NimBLEHandler::pServer = nullptr;

// void MyServerCallbacks::onConnect(NimBLEServer* pServer) {
//     NimBLEHandler::deviceConnected = true;
// }

// void MyServerCallbacks::onDisconnect(NimBLEServer* pServer) {
//     NimBLEHandler::deviceConnected = false;
// }

// // Runs for each UUID
// NimBLECharacteristic* intializeBLECharacteristic(NimBLECharacteristic* pCharacteristic, NimBLEService* pService, const char* characteristicUUID, const char* label) {
//   pCharacteristic = pService->createCharacteristic(
//     characteristicUUID,
//     NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::READ
//   );

//   pCharacteristic->createDescriptor("2901", NIMBLE_PROPERTY::READ)->setValue(label);

//   return pCharacteristic;
// }

// // Runs one time to setup
// void NimBLEHandler::setupBLE(std::string deviceName, static NimBLEUUID, char* UUIDLabels[]) { // can either pass through 

//   const int NimBLEHandler::NumCharacteristics = sizeof(UUIDLabels) / sizeof(UUIDLabels[0]);

//   NimBLEDevice::init(deviceName);
//   pServer = NimBLEDevice::createServer();
//   pServer->setCallbacks(new MyServerCallbacks());

//   NimBLEService* pService = pServer->createService(BLESERVICE_UUID);

//   for (int i = 0; i < NumCharacteristics; i++) {
//     pCharacteristicChars[i] = intializeBLECharacteristic(pCharacteristicChars[i], pService, characteristicUUIDs[i], UUIDLabels[i]);
//   }

//   pService->start();

//   NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
//   pAdvertising->addServiceUUID(BLESERVICE_UUID);
//   pAdvertising->setScanResponse(false);
//   NimBLEDevice::startAdvertising();
//   Serial.println("Waiting for a client connection to notify...");
// }

// void NimBLEHandler::SendData(std::vector<std::string> EncodedData ) { 
   
//   for(int i=0; i<NumCharacteristics; i++) {
//     pCharacteristicChars[i]->setValue(std::string(EncodedData[i]));
//     pCharacteristicChars[i]->notify();
//   }

//   if (!deviceConnected && oldDeviceConnected) {
//     delay(500);
//     NimBLEDevice::startAdvertising();
//     Serial.println("start advertising");
//     oldDeviceConnected = deviceConnected;
//   }

//   if (deviceConnected && !oldDeviceConnected) {
//     Serial.println("Device Connected");
//     oldDeviceConnected = deviceConnected;
//   }
// }

// void NimBLEHandler::SendNotificationDummy() { 
   
//   std::string DummyTimestamp = "04:32:27 13/06/0023";

//   for(int i=0; i<NumCharacteristics; i++) {
//     pCharacteristicChars[i]->setValue(DummyTimestamp);
//     pCharacteristicChars[i]->notify();
//     delay(500);
//   }

//   if (!deviceConnected && oldDeviceConnected) { /// this logic may need to belong in the larger loop
//     delay(500);
//     NimBLEDevice::startAdvertising();
//     Serial.println("start advertising");
//     oldDeviceConnected = deviceConnected;
//   }

//   if (deviceConnected && !oldDeviceConnected) {
//     Serial.println("Device Connected");
//     oldDeviceConnected = deviceConnected;
//   }
// }

