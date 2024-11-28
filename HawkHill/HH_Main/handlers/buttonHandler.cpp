#include "buttonHandler.h"

void Button::setup(uint8_t irq_pin, void (*ISR_callback)(void), int value) {
  pinMode(irq_pin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(irq_pin), ISR_callback, value);
  Serial.println("Button Initialized");
}

void Button::handleInterrupt(void) {
  rightNowButtonPressTime = millis();
  if (rightNowButtonPressTime - timeSinceLastPress >= debounceTime) {
    countNumKeyPresses++;
    timeSinceLastPress = rightNowButtonPressTime;
  }
}

uint32_t Button::getCount() {
  return countNumKeyPresses;
}

void Button::resetCount() {
  countNumKeyPresses=0;
}