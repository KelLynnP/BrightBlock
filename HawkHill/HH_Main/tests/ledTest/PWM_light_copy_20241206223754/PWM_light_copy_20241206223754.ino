#include <Arduino.h>

const int pwmPin = 14;  // PWM pin connected to the EN pin of TPS61500
const int frequency = 1000; // hZ
const int resolution = 8 ; // bit
const int pwmPercent = 60;


void setup() {
  // New API: single function call to configure PWM
  // Parameters: pin, frequency (Hz), resolution (bits)

  pinMode(pwmPin, OUTPUT);
  // if (!ledcSetClockSource(LEDC_AUTO_CLK)){
  //     Serial.println("PWM Setup Failed!");
  //     return;
  // }
  // if (!ledcAttach(pwmPin, frequency, resolution)  ) {
  //   Serial.println("PWM Setup Failed!");
  //   return;
  // }


  Serial.begin(115200);
  Serial.println("PWM Setup Complete");
}

void loop() {

  // digitalWrite(pwmPin, HIGH);
  // delay(5000);
  // digitalWrite(pwmPin, LOW);
  // delay(2000);

  analogWrite(pwmPin,pwmPercent*255);
  // ledcWrite(pwmPin, 200);  // 255 == 100 % duty cycle 
  Serial.println(sprintf("PWM at 10 %d");
  stringcmp()
  delay(5000);


  // ledcWrite(pwmPin, 0);  // 50% duty cycle
  // Serial.println("PWM at 0%");
  // delay(1000);


}

  // pinMode(pwmPin, OUTPUT);

  // // With 8-bit resolution, 128 still gives us 50% duty cycle
  // analogWrite(pwmPin, 128);
  // delay(1000);

   // analogWrite(pwmPin, 0);
  // delay(1000);
  