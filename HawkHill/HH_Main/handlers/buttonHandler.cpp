#include "buttonHandler.h"

void Button::setup(uint8_t irq_pin, void (*ISR_callback)(void), int value) {
  pinMode(irq_pin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(irq_pin), ISR_callback, value);
  Serial.println("Button Initialized");
}

void Button::handleInterrupt(void) { // Button state has changed
  Button::rightNowButtonPressTime = millis();
  if (Button::rightNowButtonPressTime - Button::timeSinceLastPress >= Button::debounceTime) { // if we're catching a real press
    // buttonPressed = !buttonPressed; // Invert the button state
    Button::lastEdge = digitalRead(logButtonPin);

    if (Button::lastEdge) { // if button is being pressed
      Button::pressStartTime = Button::rightNowButtonPressTime;
    } 
    else {
      // release cases
      uint32_t pressDuration = Button::rightNowButtonPressTime - Button::pressStartTime;
      
      if (pressDuration >= LONG_PRESS_DURATION ) { // if button is pressed for longer than LONG_PRESS_DURATION and the button is being released
        longPressCount++;
      } else { // if we're catching the release of a short press
        shortPressCount++;
      }
      timeSinceLastPress = millis();
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