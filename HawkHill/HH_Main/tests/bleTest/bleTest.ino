#include <NimBLEDevice.h>
#include "../../handlers/bleHandler.h"
#include "../../handlers/bleHandler.cpp" 
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <map>

// Write Actions
// 0 = Don’t send data
// 1 = BroadCast
// 2 = Run Transmit 

// Function declarations
void sendSimBulkData(int numberOfDataPoints = 5);
void sendErrorState();
void sendSensorData();

enum dataType {
    timestamp,
    lat,
    lon,
    alt,
    fix,
    pm25,
    temp,
    near,
    mode,
    bat, 
    error
};



// Function to generate random data
std::string simDataGenerator(dataType Sim) {
    float start; float end; int precision;
    switch (Sim) {
        case dataType::timestamp: {
            // Generate a random time of day
            int hours = rand() % 24;
            int minutes = rand() % 60;
            int seconds = rand() % 60;

        char buffer[9];  // HH:MM:SS + null terminator
        sprintf(buffer, "%02d:%02d:%02d", hours, minutes, seconds);
            return std::string(buffer);
        }
        case dataType::lat: { 
            start = -90; end = 90; precision = 5;
        }
        case dataType::lon: {
            start = -180; end = 180; precision = 5;
        }
        case dataType::alt: {
            start = 0; end = 1000; precision = 2;
        }
        case dataType::pm25: {
            start = 0; end = 100; precision = 1;
        }
        case dataType::temp: {
            start = 0; end = 100; precision = 1;
        }
        case dataType::near: {
            start = 0; end = 1; precision = 1;
        }
        case dataType::mode: {
            start = 0; end = 1; precision = 1;
        }
        case dataType::bat: {
            start = 0; end = 100; precision = 2;
        }
        case dataType::error: {
            start = 0; end = 1; precision = 1;
        }
    }
    // Generate a random float
    float scale = pow(10, precision);
    float randomValue = start + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (end - start)));
    float roundedValue = round(randomValue * scale) / scale;
    return std::string(roundedValue, precision);
}

// Define test data structure first
struct SensorData {
    // Write characteristics (from client)
    std::string modeType = "0";        
    std::string errorString = "0";    
    
    // Notify characteristics (to client)
    const char* timeStamp = "12:34:56";
    const char* latitude = "51.5074";
    const char* longitude = "-0.1278";
    const char* altitude = "100.5";
    const char* gpsFix = "1";
    const char* pm25 = "12.3";
    const char* temperature = "25.6";
    const char* nearMissEvent = "0";
    const char* modeButtonEvent = "0";
    const char* batteryLevel = "85";

    // Add array-style accessor
    const char* operator[](dataType type) const {
        switch(type) {
            case timestamp: return timeStamp;
            case lat: return latitude;
            case lon: return longitude;
            case alt: return altitude;
            case fix: return gpsFix;
            case pm25: return pm25;
            case temp: return temperature;
            case near: return nearMissEvent;
            case mode: return modeButtonEvent;
            case bat: return batteryLevel;
            case error: return errorString.c_str();
            default: return "0";
        }
    }
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

const std::map<dataType, const char*> characteristicLabels = {
    {mode, "ModeType"},
    {error, "ErrorString"},
    {timestamp, "TimeStamp"},
    {lat, "Latitude"},
    {lon, "Longitude"},
    {alt, "Altitude"},
    {fix, "GPSFix"},
    {pm25, "PM25"},
    {temp, "Temperature"},
    {near, "NearMissEvent"},
    {mode, "ModeButtonEvent"},
    {bat, "BatteryLevel"}
};

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
                sendSimBulkData(5); // General non BLE specific helper function
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
    for (const auto& [type, label] : characteristicLabels) {
        bleHandler.updateCharacteristic(label, testData[type]);
    }
}

void sendSimSensorData() {
    std::map<dataType, std::string> sensorData;
    
    // Generate all sensor data
    for (const auto& [type, label] : characteristicLabels) {
        sensorData[type] = simDataGenerator(type);
    }
    
    // Update all characteristics
    for (const auto& [type, label] : characteristicLabels) {
        bleHandler.updateCharacteristic(label, sensorData[type].c_str());
    }
}

void sendSimBulkData(int numberOfDataPoints) {
    std::map<dataType, std::string> sensorData;
    
    // Generate all sensor data
    for (const auto& [type, label] : characteristicLabels) {
        sensorData[type] = simDataGenerator(type);
    }
    
    String jsonData = "[";
    
    for (int i = 0; i < numberOfDataPoints; i++) {
        if (i == 0) {
            for (const auto& [type, label] : characteristicLabels) {
                jsonData += "\"" + String(label) + "\":" + String(sensorData[type].c_str()) + ",";
            }
        }
        else {
            for (const auto& [type, label] : characteristicLabels) {
                if (type == timestamp) {  // Skip adding noise to time values
                    // Parse the time string (format: "HH:MM:SS")
                    std::string timeStr = sensorData[type];
                    int seconds = std::stoi(timeStr.substr(6, 2));  // Get SS
                    seconds = (seconds + i) % 60;  // Add increment and wrap around
                    
                    // Create new timestamp with incremented seconds
                    std::string newTimestamp = timeStr.substr(0, 6) + 
                                              (seconds < 10 ? "0" : "") + 
                                              std::to_string(seconds);
                    
                    jsonData += "\"" + String(label) + "\":" + String(newTimestamp.c_str()) + ",";
                    continue;
                }
                try {
                    float baseValue = std::stof(sensorData[type]);
                    float noise = (rand() % 100 - 50) / 1000.0f;
                    float newValue = baseValue + noise;
                    jsonData += "\"" + String(label) + "\":" + String(newValue, 6) + ",";
                } catch (...) {
                    // If conversion fails, use original value
                    jsonData += "\"" + String(label) + "\":" + String(sensorData[type].c_str()) + ",";
                }
            }
        }
        // Add comma between objects, but not after the last one
        if (i < numberOfDataPoints - 1) {
            jsonData += ",";
        }
    }
    
    jsonData += "]";
    
    // Debug print before sending
    Serial.println("Json Data Created:");
    Serial.println(jsonData);

    Serial.println("Transmitting Data...");
    bleHandler.transmitBulkData(jsonData.c_str());
}

