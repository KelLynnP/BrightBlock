#include "../../../handlers/ledHandler.h"
#include "../../../handlers/ledHandler.cpp"

LED led(LED::LEDType::INDICATOR);

void setup() {
  Serial.begin(115200);
  led.setIndicator(LED::IndicatorPattern::FOLLOW_POLLUTION);

}

void loop() {

for (int i = 0; i < 400; i++) {
    led.update(i);
    delay(10);
  }

}

