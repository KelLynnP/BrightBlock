#include "../../handlers/ledHandler.h"

const int LED_PIN = 16;    // PWM pin connected to the EN pin of TPS61500
const int LED_CHANNEL = 0; // PWM channel

LED highCurrentLed(LED_PIN, LED_CHANNEL);

void setup() {
    Serial.begin(115200);
    Serial.println("High Current LED Test Starting...");
}

void loop() {
    // Test different brightness levels
    highCurrentLed.setBrightness(64);   // 25% brightness
    delay(1000);
    highCurrentLed.setBrightness(128);  // 50% brightness
    delay(1000);
    highCurrentLed.setBrightness(192);  // 75% brightness
    delay(1000);
    highCurrentLed.setBrightness(255);  // 100% brightness
    delay(1000);
}