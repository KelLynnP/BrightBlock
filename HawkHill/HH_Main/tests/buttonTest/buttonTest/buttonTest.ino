#include "../../../handlers/buttonHandler.h"
#include "../../../handlers/buttonHandler.cpp"

Button* logEventButton = new Button();
Button* modeButton = new Button();
void setup() {
    Serial.begin(115200);
    logEventButton->setup(Button::logButtonPin, []{ logEventButton->handleInterrupt(); }, CHANGE);
    modeButton->setup(Button::modeButtonPin, []{ modeButton->handleInterrupt(); }, CHANGE);
}

void loop() {
  Serial.println(modeButton->getCount());
  delay(1000);

  if(modeButton->getCount() > 10) {
    Serial.println("Button pressed 10 times");
    modeButton->resetCount();
  }
}