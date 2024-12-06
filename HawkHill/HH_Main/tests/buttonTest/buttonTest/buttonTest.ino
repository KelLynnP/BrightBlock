#include "../../../handlers/buttonHandler.h"
#include "../../../handlers/buttonHandler.cpp"

// Button* logEventButton = new Button();
Button* modeButton = new Button();
bool isHigh = digitalRead(Button::modeButtonPin) == HIGH;


void setup() {
    Serial.begin(115200);
    
    pinMode(Button::modeButtonPin, INPUT_PULLUP);
    attachInterrupt(
        digitalPinToInterrupt(Button::modeButtonPin), 
        modeButton->handleInterrupt, 
        FALLING
    );
    
    Serial.println("Change interrupt initialized");
}

void loop() {
    Serial.print("Raw:");
    Serial.print(digitalRead(Button::modeButtonPin));
    Serial.print(",");
    Serial.print("Interrupt:");
    Serial.println(modeButton->getLastEdgeWasRising() ? 1 : 0);
    delay(10);
}