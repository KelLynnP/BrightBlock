#ifndef NIMBLEHANDLER_H
#define NIMBLEHANDLER_H

#include <NimBLEDevice.h>
#include <unordered_map>
#include <string>

class MyServerCallbacks : public NimBLEServerCallbacks {
public:
    void onConnect(NimBLEServer* pServer) override;
    void onDisconnect(NimBLEServer* pServer) override;
};

class NimBLEHandler {
private:

    static const int NumCharacteristics;
    NimBLECharacteristic* pCharacteristicChars[];
    static bool deviceConnected;
    static bool oldDeviceConnected;
    static NimBLEServer* pServer;
    
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

public:
    void setupBLE(std::string deviceName, );
};

#endif // NIMBLEHANDLER_H
