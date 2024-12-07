#include "buttonHandler.h"


void Button::handleInterrupt() {
    uint32_t now = millis();
    if (now - lastInterruptTime >= debounceTime) {
        bool pinState = digitalRead(pin);
        debugInfo.pinState = pinState;
        debugInfo.lastInterruptTime = now;
        
        if (!pinState) {  // Rising edge
            riseTime = now;
            debugInfo.isRise = true;
        } else {         // Falling edge
            fallTime = now;
            debugInfo.isRise = false;
            lastInterruptTime = now;
            debugInfo.lastPressTime = now;
            uint32_t pressTime = fallTime - riseTime;
            
            if (pressTime >= longPressDuration) {
                longPressCount++;
            } else if (pressTime > debounceTime) {
                shortPressCount++;
            }
        } 
    }
}

uint32_t Button::getLongPressCount() {
    return longPressCount;
}

uint32_t Button::getShortPressCount() {
    return shortPressCount;
}

void Button::resetCount() {
    shortPressCount = 0;
    longPressCount = 0;
}

Button::PressCount Button::getButtonCount() {
    if (longPressCount > 0) {
        return {PressType::LONG_PRESS, longPressCount};
    } else if (shortPressCount > 0) {
        return {PressType::SHORT_PRESS, shortPressCount};
    }
    return {PressType::NONE, 0};
}

