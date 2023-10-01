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
#include <StateMachine.h>

// const int STATE_DELAY = 1000; // this logic is cursed
// int randomState = 0;
// const int LED = 13;

StateMachine machine = StateMachine(); // declare instance


State* idle = machine.addState(&idleS); // starting state? Any way to declare this as the main one?
State* dataTakingMode = machine.addState(&dataTakingState);


void setup() {
  Serial.begin(115200);

  // enable UI components

  // set up sensors (GPS + Sensirion) 

  // Set up memory card

  // set states

  // set transtiions 
  idle->addTransition(&transitionidle2dataTakingMode,dataTakingMode);
  dataTakingMode->addTransition(&transitiondataTakingMode2idle,idle); // transitions return the state they are heading to?
  dataDumpMode->addTransition(&transition,S4);


void loop() {
  machine.run();
  delay(STATE_DELAY);
}


//=======================================

void idleS(){

// waiting for left button to be pressed // (so this is transition state)

// light does not blink // 

// probably run GPS in order to so that its ready to go for later! 

}

bool transitionidle2dataTakingMode(){
  return true;
}

// Transitions are evaluated by the state machine after the state logic has executed. If none of the transitions evaluate to true, then the machine stays in the current state.

//-------------------------
void dataTakingState(){

  // light blinks saying data is good 


}

bool transitiondataTakingMode2idle() {

}
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
}
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