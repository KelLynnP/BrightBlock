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

    static void IRAM_ATTR staticISR(void* arg) {
        Button* button = (Button*)arg;
        button->handleInterrupt();
    }

  public:
    Button(uint8_t buttonPin) : pin(buttonPin) {
        Serial.print("Initializing button on pin: ");
        Serial.println(pin);
        
        pinMode(pin, INPUT);
        attachInterruptArg(digitalPinToInterrupt(pin), 
                          staticISR,
                          this, 
                          CHANGE);
        
        Serial.println("Button initialized with interrupt");
        Serial.print("Initial pin state: ");
        Serial.println(digitalRead(pin));
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
        uint32_t time;
    };

    struct DebugInfo {
        uint32_t lastInterruptTime = 0;
        uint32_t lastPressTime = 0;
        bool pinState = 0;
        bool isRise = 0;
    }; DebugInfo debugInfo;

    PressCount getButtonCount();
    uint32_t getLongPressCount();
    uint32_t getShortPressCount();

    static const uint8_t logButtonPin = 14;  
    static const uint8_t modeButtonPin = 21;
};

#endif //BUTTONHANDLER_H
   