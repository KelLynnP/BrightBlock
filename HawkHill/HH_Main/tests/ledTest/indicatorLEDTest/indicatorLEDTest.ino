#include "../../../handlers/ledHandler.h"
#include "../../../handlers/ledHandler.cpp"

// const int frequency = 1000; // hZ
// const int resolution = 8 ; // bit
const float pwmPercentHigh = 0; // %
const float pwmPercentLow = 1; // %
int highValue = int(pwmPercentHigh * 255);
int lowValue = int(pwmPercentLow * 255);

// const int indicatorGreenPin = 25;
// const int indicatorRedPin = 26;
// const int indicatorBluePin = 27;

void setup() {
  Serial.begin(115200);
  
  if (!ledcAttach(LED::indicatorRedPin, frequency, resolution)) {
    Serial.println("Red Setup Failed!");
  }
  if (!ledcAttach(LED::indicatorBluePin, frequency, resolution)) {
    Serial.println("Blue Setup Failed!");
  }
  if (!ledcAttach(LED::indicatorGreenPin, frequency, resolution)) {
    Serial.println("Green Setup Failed!");
  }

}

void loop() {
  // put your main code here, to run repeatedly
  ledcWrite(indicatorRedPin,highValue);
  ledcWrite(indicatorGreenPin,lowValue);
  ledcWrite(indicatorBluePin,lowValue);
  delay(2000);

  ledcWrite(indicatorRedPin, lowValue);
  ledcWrite(indicatorGreenPin, lowValue);
  ledcWrite(indicatorBluePin, lowValue);

  delay(2000);
}
// LED beaconLED(LED::LEDType::BEACON);
// LED::BeaconPattern.setupIndicator beaconPattern = LED::BeaconPattern::HIGH;
// LED indicatorLED(LED::LEDType::INDICATOR);
// LED::IndicatorPattern indicatorPattern = LED::IndicatorPattern::STARTUP;
// indicatorLED.setupIndicator(indicatorPattern);


  // int start = 0;
  // switch (start) {
  //   case 0:
  //     ledcWrite(LED::indicatorRedPin, highValue);
  //     ledcWrite(LED::indicatorGreenPin, lowValue);
  //     ledcWrite(LED::indicatorBluePin, lowValue);
  //     start = 1;

  //     delay(2000);
  //   case 1:
  //     ledcWrite(LED::indicatorRedPin, lowValue);
  //     ledcWrite(LED::indicatorGreenPin, highValue);
  //     ledcWrite(LED::indicatorBluePin, lowValue);
  //     start = 2;

  //     delay(2000);
  //   case 2:
  //     ledcWrite(LED::indicatorRedPin, lowValue);
  //     ledcWrite(LED::indicatorGreenPin, lowValue);
  //     ledcWrite(LED::indicatorBluePin, highValue);
  //     start = 3;

  //     delay(2000);
  //   case 3:
  //     ledcWrite(LED::indicatorRedPin, highValue);
  //     ledcWrite(LED::indicatorGreenPin, highValue);
  //     ledcWrite(LED::indicatorBluePin, highValue);
  //     start = 0;

  //     delay(2000);
  //   default:
  //     ledcWrite(LED::indicatorRedPin, lowValue);
  //     ledcWrite(LED::indicatorGreenPin, lowValue);
  //     ledcWrite(LED::indicatorBluePin, lowValue);
  //     start = 0;
      
  //     delay(2000);
  // }

