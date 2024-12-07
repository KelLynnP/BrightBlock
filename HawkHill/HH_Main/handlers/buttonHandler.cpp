#include "buttonHandler.h"

void Button::setup(uint8_t irq_pin, void (*rising_callback)(void), void (*falling_callback)(void)) {
  attachInterrupt(digitalPinToInterrupt(irq_pin), falling_callback, CHANGE);
  Serial.println("Button Initialized");
}

void Button::handleInterrupt() {
    uint32_t now = millis();
    if (now - lastInterruptTime >= debounceTime) {
        bool pinState = GPIO.in & (1 << pin);  // Use instance pin instead of static pin
        
        if (pinState) {  // Rising edge
            riseTime = now;
        } else {         // Falling edge
            fallTime = now;
            uint32_t pressTime = fallTime - riseTime;
            
            if (pressTime >= longPressDuration) {
                longPressCount++;
            } else if (pressTime > debounceTime) {  // Ensure it's a real press
                shortPressCount++;
            }
        }
        
        lastInterruptTime = now;
    }
}

void Button::setCounts() {

uint32_t Button::getLongPressCount() {
  return longPressCount;
}

uint32_t Button::getShortPressCount() {
  return shortPressCount;
}

void Button::resetCount() {
  shortPressCount=0;
  longPressCount=0;
}

Button::PressCount Button::getButtonCount() {
    if (longPressCount > 0) {
        return {PressType::LONG_PRESS, longPressCount};
    } else if (shortPressCount > 0) {
        return {PressType::SHORT_PRESS, shortPressCount};
    }
    return {PressType::NONE, 0};
}

