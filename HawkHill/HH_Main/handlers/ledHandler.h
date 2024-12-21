#ifndef LED_H
#define LED_H

#include <Arduino.h>
#include
class LED {
private: 
    const int pin;
    const int channel;
    static const int FREQUENCY = 1000;    // 1 KHz
    static const int RESOLUTION = 8;      // 8-bit resolution (0-255)

public:
    LED(int ledPin, int pwmChannel) : pin(ledPin), channel(pwmChannel) {
        ledcSetup(channel, FREQUENCY, RESOLUTION);
        ledcAttachPin(pin, channel);
    }
    
    void setBrightness(uint8_t brightness) {
        ledcWrite(channel, brightness);
    }
};

#endif