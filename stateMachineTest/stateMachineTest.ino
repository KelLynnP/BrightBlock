
#include <StateMachine.h>
#include <iostream>
#include <string>

// this should all go into a header file ahaha :)
// BUTTON STUFF
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

// LED STUFF
class LED {
private:
  std::string state;
  uint32_t LEDC_CHANNEL_0;  // #FIXME -> channels can change (dig plz)
  uint32_t LEDC_TIMER_12_BIT = 12;
  uint32_t LEDC_BASE_FREQ;
  uint32_t lastToggleTime;

public:
  int LedPin;
  LED(int LedPinAssigned, int ChannelSet)
    : LedPin(LedPinAssigned), LEDC_CHANNEL_0(LEDC_CHANNEL_0) {
    LEDC_BASE_FREQ = 5000;  // use 5000 Hz as a LEDC base frequency// could change based on LED

    lastToggleTime = millis();
    state = "low";
    ledcSetup(LEDC_CHANNEL_0, LEDC_BASE_FREQ, LEDC_TIMER_12_BIT);
    ledcAttachPin(LedPin, LEDC_CHANNEL_0);
  }

  void ledSet(int brightnessHigh, int brightnessLow, int timingInterval) {  // neeed to add flash frequency
    if ((millis() - lastToggleTime) >= timingInterval) {
      if (state == "low") {
        ledcAnalogWrite(LEDC_CHANNEL_0, brightnessHigh);
        state = "high";
      } else {
        ledcAnalogWrite(LEDC_CHANNEL_0, brightnessLow);
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

StateMachine machine = StateMachine();  // declare instance

State* idle = machine.addState(&idleState);  // starting state? Any way to declare this as the main one?
State* dataTaking = machine.addState(&dataTakingState);
State* dataDump = machine.addState(&dataDumpState);

// ButtonSet
Button* logEventButton;
uint8_t logEventButtonPin = 14;

Button* stateButton;
uint8_t stateButtonPin = 26;

int brightHigh = 10;
int brightLow = 5;
int timeDelayMS = 1000;
int channel0 = 0 ;
int ledPin = 25;

LED StatusLED(ledPin, channel0);  //okay make an instance of this guy @pin 25, keeping channel @0


void setup() {
  Serial.begin(115200);

  // set transtiions
  idle->addTransition(&transitionidle2dataTaking, dataTaking);
  dataTaking->addTransition(&transitiondataTaking2idle, idle);  // transitions return the state they are heading to?
  // dataDumpMode->addTransition(&transition,S4);

  // Button set
  logEventButton = new Button();
  logEventButton->setup(
    logEventButtonPin, [] {
      logEventButton->handleInterrupt();
    },
    RISING);

  stateButton = new Button();
  stateButton->setup(
    stateButtonPin, [] {
      stateButton->handleInterrupt();
    },
    RISING);

  // enable UI components
  // set up sensors (GPS + Sensirion)
  // Set up memory card
}

void loop() {
  machine.run();
  delay(1000);
}
// Transitions are evaluated by the state machine after the state logic has executed. If none of the transitions evaluate to true, then the machine stays in the current state.

void idleState() {
  Serial.printf("Idle State ! Nothing 2 c here :) \n");
  Serial.printf("Just waiting until the state button is pressed to change:  %d \n", stateButton->getCount());
  StatusLED.ledSet(0, 0, timeDelayMS);


  // waiting for left button to be pressed // (so this is transition state)
  // light does not blink //
  // probably run GPS in order to so that its ready to go for later!
}

bool transitionidle2dataTaking() {
  int buttonCountForPrint = stateButton->getCount();
  if (buttonCountForPrint != 0) {
    Serial.printf("Beeep Boop time to swap state! Idle -> Data Taking  \n");
    stateButton->resetCount();
    return true;
  } 
  return false;
}

//-------------------------
void dataTakingState() {
  Serial.printf("Data time!  \n");

  // light blinks saying data is good
  StatusLED.ledSet(brightHigh, brightLow, timeDelayMS);
}

bool transitiondataTaking2idle() {
  int buttonCountForPrint = stateButton->getCount();
  if (buttonCountForPrint != 0) {
      Serial.printf("Beeep Boop time to swap state! Data Taking -> Idle \n");
      stateButton->resetCount();
      return true;
  }
    return false;
}

void dataDumpState() {
  Serial.printf("Data time!  \n");
}

bool transitionidle2dataDumpState() {
  // int buttonCountForPrint = stateButton->getCount();
  // if (buttonCountForPrint != 0) {
  //     Serial.printf("Beeep Boop time to swap state! Data Taking -> Idle \n");
  //     stateButton->resetCount();
  //     return true;
  // }
  //   return false;
}


/****************************************************************
 * 
 *                STATE MACHINE EXAMPLE SKETCH
 *            
 *
 *  This sketch is an example to learn how to use the state
 *  machine. In this example we define a state machine with
 *  6 states (S0-S5). 
 *  
 *  STATES
 *  There are two ways of declaring a state logic:
 *  1. Through a lambda function (an anonymous function) declared
 *  in the addState() method.
 *  2. Defining the function normally and passing the address 
 *  to the addState() method.
 *  
 *  States contain the machine logic of the program. The machine
 *  only evaluates the current state until a transition occurs
 *  that points to another state.
 *  
 *  To evaluate a piece of code only once while the machine 
 *  is in a particular state, you can use the machine.evaluateOnce 
 *  attribute. It is true each time the machine enters a new state 
 *  until the first transition is evaluated.
 *  
 *  TRANSITIONS
 *  Each state has transitions defined in setup(). Transitions
 *  require two parameters, 
 *  1. The transition test function that
 *  returns a boolean value indicating whether or not the 
 *  transition occured, 
 *  2. The number of the target state. The target state can also 
 *  be specified passing the state pointer. This could point to 
 *  the same state it is in, if you want to dynamically set the
 *  transition target. To do so, use state->setTransition(). You
 *  must pass the index of the transition you want to modify and
 *  the number of the target state.
 * 
 *  Transitions are evaluated by the state machine after the state
 *  logic has executed. If none of the transitions evaluate to 
 *  true, then the machine stays in the current state. 
 *  
 *  
 *  Author: Jose Rullan
 *  Date: 10/December/17
 *  Project's page: https://github.com/jrullan/StateMachine
 *  License: MIT

 
 ****************************************************************/
/*
 * Example of using a lambda (or anonymous function) callback
 * instead of providing the address of an existing function.
 * Also example of using the attribute executeOnce to execute
 * some part of the code only once per state.
//  */


/*
 * The other way to define states.
 * (Looks cleaner)
 * Functions must be defined in the sketch
 
  * State* S0 = machine.addState([](){
  *   Serial.println("State 0, anonymous function");
  *   if(machine.executeOnce){
  *     Serial.println("Execute Once");
  *     digitalWrite(LED,!digitalRead(LED));
  *   }
  * });;
  */
// State* S3 = machine.addState(&state3);
// State* S4 = machine.addState(&state4);
// State* S5 = machine.addState(&state5);

/*
  * The other way to define transitions.
  * (Looks cleaner)
  * Functions must be defined in the sketch
  */
// S4->addTransition(&transitionS4S5,S5);
// S5->addTransition(&transitionS5S0,S0);
// S5->addTransition(&transitionS5dataTakingMode,dataTakingMode);
//}
// pinMode(LED,OUTPUT);
// randomSeed(A0);

/*
   * Example of a transition that uses a lambda
   * function, and sets the transition (first one)
   * to a random state.
   * 
   * Add only one transition(index=0)
   * that points to randomly selected states
   * Initially points to itself.
   */
// S0->addTransition([](){
//   randomState = random(0,6);
//   Serial.print("Transitioning to random state ");
//   Serial.println(randomState);
//   S0->setTransition(0,randomState);
//   return true;
// },S0);

// bool transitiondataTakingModeS3(){
//   return true;
// }

// //------------------------
// void state3(){
//   Serial.println("State 3");
// }

// bool transitionS3S4(){
//   return true;
// }

// //-------------------------
// void state4(){
//   Serial.println("State 4");
// }

// bool transitionS4S5(){
//   return true;
// }

// //-------------------------
// void state5(){
//   Serial.println("State 5");
// }

// bool transitionS5S0(){
//   return random(0,2);
// }

// bool transitionS5dataTakingMode(){
//   return true;
// }