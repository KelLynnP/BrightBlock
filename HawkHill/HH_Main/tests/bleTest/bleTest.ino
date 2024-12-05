#include <NimBLEDevice.h>
#include "../../handlers/bleHandler.h"
#include "../../handlers/bleHandler.cpp" 
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <sstream>
#include <iomanip>

// Write Actions
// 0 = Don’t send data
// 1 = BroadCast
// 2 = Run Transmit 

// Function declarations
void sendBulkData(int numberOfDataPoints = 5);
void sendErrorState();
void sendSensorData();

// Function to generate random data
String simDataGenerator(float start, float end, int precision, bool isTimestamp = false) {
    if (isTimestamp) {
        // Generate a random time of day
        int hours = rand() % 24;
        int minutes = rand() % 60;
        int seconds = rand() % 60;

        char buffer[9];  // HH:MM:SS + null terminator
        sprintf(buffer, "%02d:%02d:%02d", hours, minutes, seconds);
        return String(buffer);
    } else {
        // Generate a random float
        float scale = pow(10, precision);
        float randomValue = start + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (end - start)));
        float roundedValue = round(randomValue * scale) / scale;
        return String(roundedValue, precision);
    }
}

// Define test data structure first
struct SensorData {
    // Write characteristics (from client)
    std::string modeType = "0";        // Using std::string for writeable characteristics
    std::string errorString = "0";    
    
    // Notify characteristics (to client)
    const char* timeStamp = "12:34:56"; // 
    const char* latitude = "51.5074";
    const char* longitude = "-0.1278";
    const char* altitude = "100.5";
    const char* gpsFix = "1";
    const char* pm25 = "12.3";
    const char* temperature = "25.6";
    const char* nearMissEvent = "0";
    const char* modeButtonEvent = "0";
    const char* batteryLevel = "85";
} testData;

class TestBLEHandler : public NimBLEHandler {
    void onCharacteristicWrite(const char* label, const std::string& value) override {
        if (strcmp(label, "ModeType") == 0) {
            Serial.printf("Test: Mode changed to %s\n", value.c_str());
            testData.modeType = value;  // Simple assignment
        }
        else if (strcmp(label, "ErrorString") == 0) {
            Serial.printf("Test: Error received: %s\n", value.c_str());
            testData.errorString = value;
        }
    }
};

TestBLEHandler bleHandler;

unsigned long lastUpdate = 0;
const unsigned long UPDATE_INTERVAL = 1000;

// Status codes for error string
const char* STATUS_IDLE = "0";
const char* STATUS_NORMAL = "1";
const char* STATUS_TRANSMITTING = "2";
const char* STATUS_TRANSMISSION_COMPLETE = "3";
const char* STATUS_TRANSMISSION_ERROR = "4";

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
            if (strcmp(testData.modeType.c_str(), "0") == 0) {
                // Do nothing
            } else if(strcmp(testData.modeType.c_str(), "1") == 0) {
                sendSimSensorData(); // General non BLE specific helper function
            }
            else if(strcmp(testData.modeType.c_str(), "2") == 0) {
                sendBulkData(); // General non BLE specific helper function
            }
            else {
                sendErrorState();
                Serial.println("Error 01, Invalid Mode Type");
            }
        }
        lastUpdate = currentMillis;
    }
    
    bleHandler.handleConnection();
    delay(10);
}

void sendErrorState() {
    bleHandler.updateCharacteristic("ErrorString", "1");
}

void sendSensorData() {
    bleHandler.updateCharacteristic("ModeType", testData.modeType.c_str());
    bleHandler.updateCharacteristic("ErrorString", testData.errorString.c_str());
    bleHandler.updateCharacteristic("TimeStamp", testData.timeStamp);
    bleHandler.updateCharacteristic("Latitude", testData.latitude);
    bleHandler.updateCharacteristic("Longitude", testData.longitude);
    bleHandler.updateCharacteristic("Altitude", testData.altitude);
    bleHandler.updateCharacteristic("GPSFix", testData.gpsFix);
    bleHandler.updateCharacteristic("PM25", testData.pm25);
    bleHandler.updateCharacteristic("Temperature", testData.temperature);
    bleHandler.updateCharacteristic("NearMissEvent", testData.nearMissEvent);
    bleHandler.updateCharacteristic("ModeButtonEvent", testData.modeButtonEvent);
    bleHandler.updateCharacteristic("BatteryLevel", testData.batteryLevel);
}

void sendSimSensorData() { // fix me this is cursed
    bleHandler.updateCharacteristic("ModeType", String(simDataGenerator(0, 2, 1)).c_str());
    bleHandler.updateCharacteristic("ErrorString", String(simDataGenerator(0, 90, 5)).c_str());
    bleHandler.updateCharacteristic("TimeStamp", String(simDataGenerator(0, 23, 2, true)).c_str());
    bleHandler.updateCharacteristic("Latitude", String(simDataGenerator(0, 90, 5)).c_str());
    bleHandler.updateCharacteristic("Longitude", String(simDataGenerator(-180, 180, 5)).c_str());
    bleHandler.updateCharacteristic("Altitude", String(simDataGenerator(0, 1000, 2)).c_str());
    bleHandler.updateCharacteristic("GPSFix", String(simDataGenerator(0, 1, 1)).c_str());
    bleHandler.updateCharacteristic("PM25", String(simDataGenerator(0, 100, 1)).c_str());
    bleHandler.updateCharacteristic("Temperature", String(simDataGenerator(0, 100, 1)).c_str());
    bleHandler.updateCharacteristic("NearMissEvent", String(simDataGenerator(0, 1, 1)).c_str());
    bleHandler.updateCharacteristic("ModeButtonEvent", String(simDataGenerator(0, 1, 1)).c_str());
    bleHandler.updateCharacteristic("BatteryLevel", String(simDataGenerator(0, 100, 2)).c_str());
}

void sendBulkData(int numberOfDataPoints) {
    String jsonData = "[";
    
    for (int i = 0; i < numberOfDataPoints; i++) {
        jsonData += "{";
        jsonData += "\"time\":\"" + String(simDataGenerator(0, 23, 2, true)) + "\",";
        jsonData += "\"lat\":" + String(simDataGenerator(0, 90, 5)) + ",";
        jsonData += "\"lon\":" + String(simDataGenerator(-180, 180, 5)) + ",";
        jsonData += "\"alt\":" + String(simDataGenerator(0, 1000, 2)) + ",";
        jsonData += "\"fix\":" + String(simDataGenerator(0, 1, 1)) + ",";
        jsonData += "\"pm25\":" + String(simDataGenerator(0, 100, 1)) + ",";
        jsonData += "\"temp\":" + String(simDataGenerator(0, 100, 1)) + ",";
        jsonData += "\"near\":" + String(simDataGenerator(0, 1, 1)) + ",";
        jsonData += "\"mode\":" + String(simDataGenerator(0, 1, 1)) + ",";
        jsonData += "\"bat\":" + String(simDataGenerator(0, 100, 2)) + "";
        jsonData += "}";
        
        // Add comma between objects, but not after the last one
        if (i < numberOfDataPoints - 1) {
            jsonData += ",";
        }
    }
    
    jsonData += "]";
    
    // Debug print before sending
    Serial.println("Sending JSON data:");
    Serial.println(jsonData);
    
    bleHandler.transmitBulkData(jsonData.c_str());
}

