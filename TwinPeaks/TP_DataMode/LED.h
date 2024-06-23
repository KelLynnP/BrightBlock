#ifndef LED_H
#define LED_H

#include <Arduino.h>
#include <driver/ledc.h>
#include <iostream>
#include <string>

class LED {
private: 
    std::string state;
    uint32_t LEDC_CHANNEL_0;  // #FIXME -> channels can change
    uint32_t LEDC_TIMER_12_BIT = 12; 
    uint32_t LEDC_BASE_FREQ; 
    uint32_t lastToggleTime; 

public:
    int LedPin;
    LED(int LedPinAssigned, int ChannelSet);
    void ledSet(int brightnessHigh, int brightnessLow, int timingInterval);
    void ledcAnalogWrite(uint8_t channel, uint32_t value);
};

#endif // LED_H