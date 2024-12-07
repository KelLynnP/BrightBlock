#ifndef BUTTONHANDLER_H
#define BUTTONHANDLER_H

#include <Arduino.h>

class Button {
  private:
    const uint8_t pin;
    uint32_t riseTime = 0;
    uint32_t fallTime = 0;
    uint32_t timeHigh = 0; 
    uint32_t lastInterruptTime = 0;

    uint32_t longPressCount = 0;
    uint32_t shortPressCount = 0;

    const uint32_t debounceTime = 50;
    const uint32_t longPressDuration = 1000;

  public:
    Button(uint8_t buttonPin) : pin(buttonPin) {
        pinMode(pin, INPUT);
        attachInterrupt(digitalPinToInterrupt(pin), 
                       [this]() { this->handleInterrupt(); }, 
                       CHANGE);
    }
    
    void handleInterrupt();

    void resetCount();

    enum class PressType {
      NONE,
      LONG_PRESS,
      SHORT_PRESS
    };

    struct PressCount {
      PressType type;
      uint32_t count;
    };

    PressCount getButtonCount();
    uint32_t getLongPressCount();
    uint32_t getShortPressCount();

    static const uint8_t logButtonPin = 14;  
    static const uint8_t modeButtonPin = 21;
};

#endif //BUTTONHANDLER_H
   
    // uint32_t getWhichButtonPressed();

    // void setCounts();

       // Debug flags
    // struct DebugInfo {
    //   bool lastEdgeWasRising = false;
    //   uint32_t lastPressDuration = 0;
    //   uint32_t interruptCount = 0;
    //   bool lastPressWasLong = false;
    // } debug;



        // Debug getters
    // bool getLastEdgeWasRising() { return debug.lastEdgeWasRising; }
    // uint32_t getLastPressDuration() { return debug.lastPressDuration; }
    // uint32_t getInterruptCount() { return debug.interruptCount; }
    // bool getLastPressWasLong() { return debug.lastPressWasLong; }