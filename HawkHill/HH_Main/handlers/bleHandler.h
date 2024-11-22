#ifndef NIMBLEHANDLER_H
#define NIMBLEHANDLER_H

#include <NimBLEDevice.h>
#include <string>

// Forward declaration
class NimBLEHandler;

// Define characteristic types as enum for better type safety and readability
enum class CharType {
    READ,
    NOTIFY,
    WRITE,
    WRITE_ONLY,
    READ_WRITE_NOTIFY
};

struct CharacteristicDef {
    const char* uuid;
    const char* label;
    CharType type;
};

// Add new callback class for characteristic writes
class CharacteristicCallbacks: public NimBLECharacteristicCallbacks {
private:
    NimBLEHandler* handler;
public:
    explicit CharacteristicCallbacks(NimBLEHandler* h) : handler(h) {}
    
    void onWrite(NimBLECharacteristic* pCharacteristic) override;
    void onRead(NimBLECharacteristic* pCharacteristic) override;
};

class NimBLEHandler {
private:
    static const int NumCharacteristics = 13;
    NimBLEServer* pServer = nullptr;
    NimBLECharacteristic* pCharacteristics[NumCharacteristics] = {nullptr};
    bool deviceConnected = false;
    bool oldDeviceConnected = false;
    
    // Service UUID as static const
    static constexpr const char* SERVICE_UUID = "4fafc201-1fb5-459e-8fcc-c5c9c331914b";
    
    // Characteristic definitions
    const CharacteristicDef characteristicDefs[NumCharacteristics] = {
        {"beb5483e-36e1-4688-b7f5-ea07361b26a8", "ModeType", CharType::WRITE},
        {"86cef02b-8c15-457b-b480-52e6cc0bdd8c", "ErrorString", CharType::WRITE},
        {"1c95d5e3-d8f7-413a-bf3d-7a2e5d7be87e", "TimeStamp", CharType::NOTIFY},
        {"d7d85823-5304-4eb3-9671-3e571fac07b9", "Latitude", CharType::NOTIFY},
        {"d2789cef-106f-4660-9e3f-584c12e2e3c7", "Longitude", CharType::NOTIFY},
        {"bf5a799d-26d0-410e-96b0-9ada1eb9f758", "Altitude", CharType::NOTIFY},
        {"755c7c73-b938-4a6e-a7be-2a3b8c3783d9", "GPSFix", CharType::NOTIFY},
        {"c22b405e-2b7b-4632-831d-54523e169a01", "PM25", CharType::NOTIFY},
        {"183b971a-79f5-4004-8182-31c88d910dca", "Temperature", CharType::NOTIFY},
        {"90b77f62-003d-454e-97fc-8f597b42048c", "NearMissEvent", CharType::NOTIFY},
        {"beb5483e-36e1-4688-b7f5-ea07361b26a8", "ModeButtonEvent", CharType::NOTIFY},
        {"ffdda8ad-60a2-4184-baff-5c79a2eccb8c", "BatteryLevel", CharType::NOTIFY},
        {"ee4d0296-f86c-41ef-8d0e-6cf1b016cdcc", "BulkData", CharType::NOTIFY}
    };

    // Helper function to convert CharType to NimBLE properties
    uint32_t getProperties(CharType type) const;

    // Add callback handler
    CharacteristicCallbacks* pCharCallbacks;
    
public:
    void setupBLE(const std::string& deviceName);
    void handleConnection();
    bool isConnected() const { return deviceConnected; }
    
    // Core methods for data transmission
    void updateCharacteristic(const char* label, const std::string& value);
    void transmitBulkData(const std::string& jsonData) {
        updateCharacteristic("BulkData", jsonData);
    }
    
    // Add method to handle incoming writes
    virtual void onCharacteristicWrite(const char* label, const std::string& value);
    
    friend class MyServerCallbacks;
    friend class CharacteristicCallbacks;
};

// Separate callback class declaration
class MyServerCallbacks : public NimBLEServerCallbacks {
private:
    NimBLEHandler* handler;
public:
    explicit MyServerCallbacks(NimBLEHandler* h) : handler(h) {}
    void onConnect(NimBLEServer* pServer) override;
    void onDisconnect(NimBLEServer* pServer) override;
};

#endif
