// CLASS DEFINITION
class Button {
  public:
    void setup(uint8_t irq_pin, void (*ISR_callback)(void), int value);
    void handleInterrupt(void);
    uint32_t getCount();
    void resetCount();


  private:
    uint32_t countNumKeyPresses = 0;
    unsigned long debounceTime = 250;  // ms
    unsigned long timeSinceLastPress = 0;
    unsigned long rightNowButtonPressTime = 0;
    bool boolPrint = true;  // Set a default value
};

void Button::setup(uint8_t irq_pin, void (*ISR_callback)(void), int value) {
  pinMode(irq_pin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(irq_pin), ISR_callback, value);
  Serial.println("Button Initialized");
}

void Button::handleInterrupt(void) {
  rightNowButtonPressTime = millis();
  if (rightNowButtonPressTime - timeSinceLastPress >= debounceTime) {
    countNumKeyPresses++;
    timeSinceLastPress = rightNowButtonPressTime;
  }
}

uint32_t Button::getCount() {
  return countNumKeyPresses;
}

void Button::resetCount() {
  countNumKeyPresses = 0;
}

// ButtonSet
Button* logEventButton;
uint8_t logEventButtonPin = 13;

Button* stateButton;
uint8_t stateButtonPin = 18;

void setup() {
  Serial.begin(115200);

  logEventButton = new Button();
  logEventButton->setup(logEventButtonPin, []{ logEventButton->handleInterrupt(); }, RISING);

  stateButton = new Button();
  stateButton->setup(stateButtonPin, []{ stateButton->handleInterrupt(); }, RISING);
}

void loop() {
  static uint32_t lastMillis = 0;
  if (millis() - lastMillis > 5000UL) {  // Check every 5 seconds
    
    Serial.printf("Log Button has been pressed %d times in the last 5 seconds\n", logEventButton->getCount());
    Serial.printf("State Button has been pressed %d times in the last 5 seconds\n", stateButton->getCount());
    lastMillis = millis();
  }
}