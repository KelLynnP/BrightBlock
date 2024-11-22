#include <NimBLEDevice.h>
#include "../../handlers/bleHandler.h"
#include "../../handlers/bleHandler.cpp" 

// Define test data structure first
struct SensorData {
    // Write characteristics (from client)
    const char* modeType = "0";        // Client writes this
    const char* errorString = "";      // Client writes this
    
    // Notify characteristics (to client)
    const char* timeStamp = "12:34:56";
    const char* latitude = "51.5074";
    const char* longitude = "-0.1278";
    const char* altitude = "100.5";
    const char* gpsFix = "1";
    const char* pm25 = "12.3";
    const char* temperature = "25.6";
    const char* nearMissEvent = "0";
    const char* modeButtonEvent = "1";
    const char* batteryLevel = "85";
} testData;

class TestBLEHandler : public NimBLEHandler {
    void onCharacteristicWrite(const char* label, const std::string& value) override {
        if (strcmp(label, "ModeType") == 0) {
            Serial.printf("Test: Mode changed to %s\n", value.c_str());
            testData.modeType = value.c_str();
        }
        else if (strcmp(label, "ErrorString") == 0) {
            Serial.printf("Test: Error received: %s\n", value.c_str());
            testData.errorString = value.c_str();
        }
    }
};

TestBLEHandler bleHandler;

unsigned long lastUpdate = 0;
const unsigned long UPDATE_INTERVAL = 1000;

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("Starting BLE setup...");
    bleHandler.setupBLE("Bleep");
    Serial.println("BLE Setup complete");
}

void loop() {
    unsigned long currentMillis = millis();
    
    if (currentMillis - lastUpdate >= UPDATE_INTERVAL) {
        if (bleHandler.isConnected()) {
            // Send individual sensor updates
            sendSensorData(); // General non BLE specific helper function
            // Send same data in bulk format
            sendBulkData(); // General non BLE specific helper function
        }
        lastUpdate = currentMillis;
    }
    
    bleHandler.handleConnection();
    delay(10);
}

void sendSensorData() {
    // Use testData directly instead of gpsHandler
    bleHandler.updateCharacteristic("TimeStamp", testData.timeStamp);
    bleHandler.updateCharacteristic("Latitude", testData.latitude);
    bleHandler.updateCharacteristic("Longitude", testData.longitude);
    bleHandler.updateCharacteristic("Altitude", testData.altitude);
    bleHandler.updateCharacteristic("GPSFix", testData.gpsFix);
    // ... other sensor updates ...
}

void sendBulkData() {
    // Create JSON with all sensor data
    String jsonData = "{";
    jsonData += "\"time\":\"" + String(testData.timeStamp) + "\",";
    jsonData += "\"lat\":" + String(testData.latitude) + ",";
    jsonData += "\"lon\":" + String(testData.longitude) + ",";
    jsonData += "\"alt\":" + String(testData.altitude) + ",";
    jsonData += "\"fix\":" + String(testData.gpsFix) + ",";
    jsonData += "\"pm25\":" + String(testData.pm25) + ",";
    jsonData += "\"temp\":" + String(testData.temperature) + ",";
    jsonData += "\"near\":" + String(testData.nearMissEvent) + ",";
    jsonData += "\"mode\":" + String(testData.modeButtonEvent) + ",";
    jsonData += "\"bat\":" + String(testData.batteryLevel) + ""; // No comma for last item
    jsonData += "}";
    
    bleHandler.transmitBulkData(jsonData.c_str()); // BLE Specific helper 
}

