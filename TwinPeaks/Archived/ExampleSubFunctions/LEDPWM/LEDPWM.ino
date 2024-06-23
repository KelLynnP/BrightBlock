#include <Arduino.h>
#include <driver/ledc.h>
#include <iostream>
#include <string>

class LED {
private: 
    std::string state;
    uint32_t LEDC_CHANNEL_0;  // #FIXME -> channels can change (dig plz)
    uint32_t LEDC_TIMER_12_BIT = 12; 
    uint32_t LEDC_BASE_FREQ; 
    uint32_t lastToggleTime; 

public:
    int LedPin;
    LED(int LedPinAssigned, int ChannelSet): LedPin(LedPinAssigned), LEDC_CHANNEL_0(ChannelSet) {
      LEDC_BASE_FREQ = 5000; // use 5000 Hz as a LEDC base frequency// could change based on LED
      lastToggleTime = millis(); 
      state = "low"; 
      ledcSetup(LEDC_CHANNEL_0, LEDC_BASE_FREQ, LEDC_TIMER_12_BIT);
      ledcAttachPin(LedPinAssigned, LEDC_CHANNEL_0);
    }

    void ledSet(int brightnessHigh, int brightnessLow, int timingInterval){ // neeed to add flash frequency
      if ((millis() - lastToggleTime) >= timingInterval) {
        if (state == "low"){
          ledcAnalogWrite(LEDC_CHANNEL_0, brightnessHigh);
          state = "high"; 
          // Serial.println("Writing High ");
        }else{
          ledcAnalogWrite(LEDC_CHANNEL_0, brightnessLow); 
          state = "low";
          // Serial.println("Writing Low ");
        }
        lastToggleTime = millis();
      }
    }

    void ledcAnalogWrite(uint8_t channel, uint32_t value) {
      // calculate duty, 4095 from 2 ^ 12 - 1
      uint32_t valueMax = 255;
      uint32_t duty = (4095 / valueMax) * min(value, valueMax);
      // write duty to LEDC
      ledcWrite(channel, duty); 
    }
};

int brightHigh = 30;
int brightLow = 5;
int timeDelayMS = 1000; 

LED StatusLED(25,0); //okay make an instance of this guy @pin 25, keeping channel @0

void setup() {
  Serial.begin(115200);
}

void loop() {
  StatusLED.ledSet(brightHigh, brightLow, timeDelayMS); 
}

