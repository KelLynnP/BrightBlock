#ifndef SEN55HANDLER_H
#define SEN55HANDLER_H

#include <SensirionI2CSen5x.h>
// logic to enable 5 volts to the sensor

class Sen55Handler {
public:
    Sen55Handler(uint8_t powerEnablePin = FiveVoltEnablePin);
    // ~Sen55Handler(); // Destructor
    void setup();
    bool pullData();
    float getPm2p5() const;
    float getPm10p5() const;
    float getAmbientHumidity() const;
    float getAmbientTemperature() const;
    float getVocIndex() const;
    float getNoxIndex() const;
    void initializePower();

private:
    static const uint8_t POWER_PIN = 35; // I35 pin 7
    SensirionI2CSen5x sen5x;
    struct Sen55Data {
        float massConcentrationPm1p0 = 0.0f;
        float massConcentrationPm2p5 = 0.0f;
        float massConcentrationPm4p0 = 0.0f;
        float massConcentrationPm10p0 = 0.0f;
        float ambientHumidity = 0.0f;
        float ambientTemperature = 0.0f;
        float vocIndex = 0.0f;
        float noxIndex = 0.0f;
    } data;

    char errorMessage[256];
    void printError(uint16_t error);
    void initializePower();
};

#endif // SEN55HANDLER_H
