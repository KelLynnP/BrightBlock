#ifndef BUTTONHANDLER_H
#define BUTTONHANDLER_H

#include <Arduino.h>

class Button {
  public:
    void setup(uint8_t irq_pin, void (*ISR_callback)(void), int value);
    void handleInterrupt(void);
    uint32_t getCount();
    void resetCount();
    static const uint8_t logButtonPin = 14;
    static const uint8_t modeButtonPin = 21;
    static const uint32_t LONG_PRESS_DURATION = 2000; // ms

  private:
    uint32_t shortPressCount = 0;
    uint32_t longPressCount = 0;
    unsigned long debounceTime = 250;  // ms
    unsigned long timeSinceLastPress = 0;
    unsigned long rightNowButtonPressTime = 0;
    bool boolPrint = true;  // Default value
    bool isRisingEdge = false;  // New variable to track edge type
    bool lastEdge = HIGH;
};

#endif //BUTTONHANDLER_H