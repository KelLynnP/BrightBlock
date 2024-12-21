#include <Arduino.h>

const int pwmPin = 14;  // PWM pin connected to the EN pin of TPS61500
const int frequency = 1000; // hZ
const int resolution = 8 ; // bit
const float pwmPercentHigh = .60; // %
const float pwmPercentLow = 0; // %
int highValue = int(pwmPercentHigh * 255);
int lowValue = int(pwmPercentLow * 255);

void setup() {

  // pinMode(pwmPin, OUTPUT);
  // ledcAttach(pwmPin,frequency,8);
  // if (!ledcSetClockSource(LEDC_AUTO_CLK)){
  //     Serial.println("PWM Setup Failed!");
  //     return;
  // }

  
  if (!ledcAttach(pwmPin, frequency, resolution)  ) {
    Serial.println("PWM Setup Failed!");
    return;
  }

  Serial.begin(115200);
  Serial.println("PWM Setup Complete");
}

void loop() {

  // High duty cycle
  // analogWrite(pwmPin, highValue);
  ledcWrite(pwmPin, highValue); // 
  // Serial.printf("PWM @ %.1f%%\n", pwmPercentHigh );
  // delay(3000);

  // Low duty cycle
  // analogWrite(pwmPin, lowValue);
  // ledcWrite(pwmPin, lowValue);  // 
  // Serial.printf("PWM @ %.1f%%\n", pwmPercentLow );
  // delay(1000);
}
  