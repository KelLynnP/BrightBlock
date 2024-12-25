#ifndef LED_H
#define LED_H

#include <Arduino.h>

class LED {
private: 
    const int pin;
    const int channel;
    static const int beaconFrequency = 1000;    // 1 KHz
    static const int beaconResolution = 8;      // 8-bit resolution (0-255)
    static const int indicatorFrequency = 1000;    // 1 KHz
    static const int indicatorResolution = 8;      // 8-bit resolution (0-255)

public:
    LED(int ledPin, int pwmChannel, bool isBeacon);
    void setBrightness(uint8_t brightness) {
        ledcWrite(channel, brightness);
    }
    enum class LEDType {
        BEACON,
        RED_INDICATOR,
        GREEN_INDICATOR,
        BLUE_INDICATOR
    };  
};

#endif