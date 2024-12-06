#include "buttonHandler.h"

void Button::setup(uint8_t irq_pin, void (*rising_callback)(void), void (*falling_callback)(void)) {
  pinMode(irq_pin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(irq_pin), rising_callback, RISING);
  attachInterrupt(digitalPinToInterrupt(irq_pin), falling_callback, FALLING);
  Serial.println("Button Initialized");
}

void Button::handleInterrupt(bool isRisingEdge) {
  rightNowButtonPressTime = millis();
  debug.lastEdgeWasRising = isRisingEdge;
  debug.interruptCount++;
  
  if (rightNowButtonPressTime - timeSinceLastPress >= debounceTime) {
    if (isRisingEdge) {
      pressStartTime = rightNowButtonPressTime;
      debug.lastPressDuration = 0;  // Reset duration on new press
    } 
    else if (pressStartTime > 0) {  // Only process if we've seen a rising edge
      uint32_t pressDuration = rightNowButtonPressTime - pressStartTime;
      debug.lastPressDuration = pressDuration;
        if (pressDuration >= longPressDuration) {
          longPressCount++;
          debug.lastPressWasLong = true;
        } else {
          shortPressCount++;
          debug.lastPressWasLong = false;
        }
      pressStartTime = 0;  // Reset for next press
      timeSinceLastPress = rightNowButtonPressTime;
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
