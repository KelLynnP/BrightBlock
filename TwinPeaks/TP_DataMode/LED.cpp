#include "LED.h"

LED::LED(int LedPinAssigned, int ChannelSet) : LedPin(LedPinAssigned), LEDC_CHANNEL_0(ChannelSet) {
    LEDC_BASE_FREQ = 5000; // use 5000 Hz as a LEDC base frequency
    lastToggleTime = millis(); 
    state = "low"; 
    ledcSetup(LEDC_CHANNEL_0, LEDC_BASE_FREQ, LEDC_TIMER_12_BIT);
    ledcAttachPin(LedPinAssigned, LEDC_CHANNEL_0);
}

void LED::ledSet(int brightnessHigh, int brightnessLow, int timingInterval) {
    if ((millis() - lastToggleTime) >= timingInterval) {
        if (state == "low"){
            ledcAnalogWrite(LEDC_CHANNEL_0, brightnessHigh);
            state = "high"; 
        } else {
            ledcAnalogWrite(LEDC_CHANNEL_0, brightnessLow); 
            state = "low";
        }
        lastToggleTime = millis();
    }
}

void LED::ledcAnalogWrite(uint8_t channel, uint32_t value) {
    uint32_t valueMax = 255;
    uint32_t duty = (4095 / valueMax) * min(value, valueMax);
    ledcWrite(channel, duty); 
}