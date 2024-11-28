#include "bleHandler.h"

void MyServerCallbacks::onConnect(NimBLEServer* pServer) {
    handler->deviceConnected = true;
    Serial.println("Device Connected");
}

void MyServerCallbacks::onDisconnect(NimBLEServer* pServer) {
    handler->deviceConnected = false;
    Serial.println("Device Disconnected");
}

uint32_t NimBLEHandler::getProperties(CharType type) const {
    switch (type) {
        case CharType::READ:
            return NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY;
        case CharType::WRITE:
            return NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::NOTIFY;
        default:
            return NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY;
    }
}

void NimBLEHandler::setupBLE(const std::string& deviceName) {
    Serial.println("Initializing BLE...");
    
    NimBLEDevice::init(deviceName);
    
    pServer = NimBLEDevice::createServer();
    if (!pServer) {
        Serial.println("Failed to create server!");
        return;
    }
    
    pServer->setCallbacks(new MyServerCallbacks(this));
    
    auto* pService = pServer->createService(SERVICE_UUID);
    if (!pService) {
        Serial.println("Failed to create service!");
        return;
    }
    
    pCharCallbacks = new CharacteristicCallbacks(this);
    
    for(int i = 0; i < NumCharacteristics; i++) {
        const auto& def = characteristicDefs[i];
        Serial.printf("Creating characteristic %s\n", def.label);
        
        pCharacteristics[i] = pService->createCharacteristic(
            def.uuid,
            getProperties(def.type)
        );
        
        if (!pCharacteristics[i]) {
            Serial.printf("Failed to create characteristic %s!\n", def.label);
            return;
        }
        
        pCharacteristics[i]->createDescriptor("2901", 
            NIMBLE_PROPERTY::READ)->setValue(def.label);
        
        if (def.type == CharType::WRITE) {
            pCharacteristics[i]->setCallbacks(pCharCallbacks);
        }
    }
    
    pService->start();
    
    auto* pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(false);
    NimBLEDevice::startAdvertising();
    
    Serial.println("BLE Setup Complete");
}

void NimBLEHandler::handleConnection() {
    if (!deviceConnected && oldDeviceConnected) {
        delay(500);
        NimBLEDevice::startAdvertising();
        Serial.println("Start advertising");
        oldDeviceConnected = deviceConnected;
    }
    
    if (deviceConnected && !oldDeviceConnected) {
        oldDeviceConnected = deviceConnected;
    }
}

void NimBLEHandler::updateCharacteristic(const char* label, const std::string& value) {
    if (!deviceConnected) return;
    
    for(int i = 0; i < NumCharacteristics; i++) {
        if (strcmp(characteristicDefs[i].label, label) == 0) {
            if (pCharacteristics[i]) {
                pCharacteristics[i]->setValue(value);
                pCharacteristics[i]->notify();
                Serial.printf("Updated %s: %s\n", label, value.c_str());
            }
            break;
        }
    }
}

void CharacteristicCallbacks::onWrite(NimBLECharacteristic* pCharacteristic) {
    std::string value = pCharacteristic->getValue();
    
    // Find the label for this characteristic
    for(int i = 0; i < handler->NumCharacteristics; i++) {
        if (handler->pCharacteristics[i] == pCharacteristic) {
            const char* label = handler->characteristicDefs[i].label;
            Serial.printf("Client wrote to %s: %s\n", label, value.c_str());
            handler->onCharacteristicWrite(label, value);
            break;
        }
    }
}

void CharacteristicCallbacks::onRead(NimBLECharacteristic* pCharacteristic) {
    // We can leave this empty if we don't need special read handling
    // The default read behavior will use the characteristic's current value
}

void NimBLEHandler::onCharacteristicWrite(const char* label, const std::string& value) {
    // Handle different characteristics
    if (strcmp(label, "ModeType") == 0) {
        // Handle mode change from client
        Serial.printf("Mode changed to: %s\n", value.c_str());
        // Add your mode handling code here
    }
    else if (strcmp(label, "ErrorString") == 0) {
        // Handle error string from client
        Serial.printf("Error received: %s\n", value.c_str());
        // Add your error handling code here
    }
}

void NimBLEHandler::transmitBulkData(const std::string& jsonData) {
    // Clear any existing data first
    updateCharacteristic("BulkData", "");
    delay(100);  // Give time for clear to process
    
    updateCharacteristic("ErrorString", STATUS_TRANSMITTING);
    
    size_t dataLength = jsonData.length();
    size_t offset = 0;
    const size_t MAX_BLE_PACKET_SIZE = 256;  // Increased for fewer chunks
    
    while (offset < dataLength) {
        if (!isConnected()) {
            updateCharacteristic("ErrorString", STATUS_TRANSMISSION_ERROR);
            updateCharacteristic("BulkData", "");
            return;
        }
        
        size_t chunkSize = std::min(MAX_BLE_PACKET_SIZE, dataLength - offset);
        std::string chunk = jsonData.substr(offset, chunkSize);
        
        updateCharacteristic("BulkData", chunk);
        offset += chunkSize;
        
        delay(300);  // More time between chunks
    }
    
    updateCharacteristic("ErrorString", STATUS_TRANSMISSION_COMPLETE);
    delay(300);
    updateCharacteristic("ErrorString", STATUS_NORMAL);
}

