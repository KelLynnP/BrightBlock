#include "../../handlers/buttonHandler.h"
#include "../../handlers/buttonHandler.cpp"

Button* modeButton;
Button* logButton;

void setup() {
    Serial.begin(115200);
    while (!Serial) delay(10);
    Serial.println("\nButton Test Starting...");
    
    modeButton = new Button(Button::modeButtonPin);
    logButton = new Button(Button::logButtonPin);

    Serial.println("Setup complete");
    Serial.println("Monitoring button presses...");
    Serial.println("Format: Short Presses, Long Presses");
}

void loop() {
    static uint32_t lastPrint = 0;
    uint32_t now = millis();
    
    // Print counts every 500ms if they're non-zero
    if (now - lastPrint >= 500) {
        Button::PressCount pressCount = logButton->getButtonCount();
        
        if (pressCount.type != Button::PressType::NONE) {
            Serial.print(pressCount.type == Button::PressType::SHORT_PRESS ? "Short: " : "Long: ");
            Serial.println(pressCount.count);
            logButton->resetCount();
        }
        
        lastPrint = now;
    }
}