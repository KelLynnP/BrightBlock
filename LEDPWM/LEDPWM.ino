
#include <iostream>
#include <string>

//  Make a class of button where it can refernce its own parameters:
class LED {
private: 
    std::string state;
    uint32_t LEDC_CHANNEL_0;  // #FIXME -> channels can change
    uint32_t LEDC_TIMER_12_BIT; 
    uint32_t LEDC_BASE_FREQ; 
    uint32_t lastToggleTime; 
public:
    int LedPin;
    LED(int LedPinAssigned, int ChannelSet): LedPin(LedPinAssigned), LEDC_CHANNEL_0(LEDC_CHANNEL_0) {
      state = "low"; //declairing an instance and setting it to low //flag Idk if this is the right call !!
      // LEDC_CHANNEL_0 = 0;
      LEDC_TIMER_12_BIT = 12;
      LEDC_BASE_FREQ = 5000; // use 5000 Hz as a LEDC base frequency
      // LedPin = LedPinAssigned;
      lastToggleTime = millis(); 

      ledcSetup(LEDC_CHANNEL_0, LEDC_BASE_FREQ, LEDC_TIMER_12_BIT);
      ledcAttachPin(LedPin, LEDC_CHANNEL_0);
    }

    void ledSet(int brightnessHigh, int brightnessLow, int timingInterval){ // neeed to add flash frequency
      if ((millis() - lastToggleTime) >= timingInterval) {
        if (state == "low"){
          ledcAnalogWrite(LEDC_CHANNEL_0, brightnessHigh); // set up 
          state = "high";
        }
        else{
          ledcAnalogWrite(LEDC_CHANNEL_0, brightnessLow); // set up 
          state = "low";
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

LED StatusLED(14,0); //okay make an instance of this guy @pin 14

void setup() {
  // Setup timer and attach timer to a led pin
}

void loop() {
  StatusLED.ledSet(10, 5, 500);
}

