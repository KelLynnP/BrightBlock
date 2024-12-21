#include "../../handlers/sen55Handler.h"
#include "../../handlers/sen55Handler.cpp"

Sen55Handler sen55;

void setup() {
    Serial.begin(115200);
    // while (!Serial) delay(10);
    sen55.enable5VPower();
    Serial.println("Setup complete");
    // pinMode(33, OUTPUT);
    // digitalWrite(33, HIGH);
    delay(1000);
}

void loop() {

  // sen55.enable5VPower();
  // delay(5000);
  // Serial.println("5V Power Enabled");


  // sen55.disable5VPower();
  // delay(5000);
  // Serial.println("5V Power Disabled");

}
