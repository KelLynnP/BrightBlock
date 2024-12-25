#include "ledHandler.h"

LED::LED(int ledPin, int ledChannel, uint32_t freq, uint32_t res) 
    : pin(ledPin), channel(ledChannel), frequency(freq), resolution(res) {
    setupPWM();
}

void LED::setupPWM() {
    ledcSetup(channel, frequency, resolution);
    ledcAttachPin(pin, channel);
    state = "low";
    lastToggleTime = millis();
}

void LED::setBrightness(uint32_t brightness) {
    ledcAnalogWrite(brightness);
}

void LED::ledcAnalogWrite(uint32_t value) {
    uint32_t maxValue = (1 << resolution) - 1;
    uint32_t duty = (maxValue / 255) * min(value, (uint32_t)255);
    ledcWrite(channel, duty);
}

void LED::toggleStateBeacon() {
    toggleState = (toggleState + 1) % 3;
    switch (toggleState) {
        case 0: setBrightness(0); break;
        case 1: setBrightness(128); break;
        case 2: setBrightness(255); break;
    }
}

void LED::setIndicator(IndicatorColor color) {
    auto pins = getIndicatorPins();  // Use the member function
    auto it = pins.find(color);
    if (it != pins.end()) {
        pin = it->second;
        frequency = INDICATOR_FREQ;
        resolution = INDICATOR_RESOLUTION;
        setupPWM();
    }
}

void LED::ledSet(int brightnessHigh, int brightnessLow, int timingInterval) {
    if ((millis() - lastToggleTime) >= timingInterval) {
        if (state == "low") {
            setBrightness(brightnessHigh);
            state = "high";
        } else {
            setBrightness(brightnessLow);
            state = "low";
        }
        lastToggleTime = millis();
    }
}
