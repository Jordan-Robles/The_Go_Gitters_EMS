/*
EMS Lab 2 Group 2

*/

#include <Arduino.h>
#include <DFRobot_BMI160.h>
#include <adxl.h>
#include <selfTest.h>
#include <stepCounting.h>


adxl accel; // Creates the accelerometer object
selfTest testInstance(accel); // Creates the selfTest instance with the accelerometer

//Setting input pins for the buttons
const int nextButton = 2; // Mode = toggles between each sub routine 
const int actionButton = 3; // Next = insdie each subroutine we can flicker between different data to be displayed
const int previousButton = 4; //Action button = COnfirms mode selection, start and stop

//button bools
bool nextPressed = false;
bool actionPressed = false;
bool previousPressed = false;

//button debouncing

const unsigned long debounceDelay = 50;

bool nextStableState = HIGH;
bool nextLastReading = HIGH;
unsigned long nextLastChangeTime = 0;

bool actionStableState = HIGH;
bool actionLastReading = HIGH;
unsigned long actionLastChangeTime = 0;

bool previousStableState = HIGH;
bool previousLastReading = HIGH;
unsigned long previousLastChangeTime = 0;


//Setting pins for LED
const int led1 = 5;
const int led2 = 6;
const int led3 = 7;

//Setting pins for the GC9A01
const int DIN = A4;
const int CLK = A5;
const int CS = 9;
const int DC = 10;
const int RES = 8;



//Time
unsigned long currentTime =0;
unsigned long previousTime =0;

//state
int currentCase = 0; // Set case = 0 to initialise system to starting profile
int subState = 0; //this is used for handling smaller sub states in the main switch cases

//debugging
bool printed = false;

//st pin stuff
int axisWorking[3] = {0,0,0};

bool readButtonDebounced(int pin, bool &stableState, bool &lastReading, unsigned long &lastChangeTime) {
  bool reading = digitalRead(pin);

  if (reading != lastReading) {
    lastChangeTime = millis();
    lastReading = reading;
  }

  if (millis() - lastChangeTime > debounceDelay) {
    stableState = reading;
  }

  return stableState;
}



void setup() {
  Serial.begin(9600); 
}

void loop() {
  switch(currentCase){

    
    case 0: //home

      //Reading state of buttons
      actionPressed = readButtonDebounced(actionButton, actionStableState, actionLastReading, actionLastChangeTime);
      nextPressed = readButtonDebounced(nextButton, nextStableState, nextLastReading, nextLastChangeTime);
      previousPressed = readButtonDebounced(previousButton, previousStableState, previousLastReading, previousLastChangeTime);

      //Printing welcome stuff to Screen
      //atm its just serial prints
      if(!printed){
        Serial.println("Welcome user");
        Serial.println("Press next to go to self test");
        Serial.println("Press back to go to step tracking");
        printed = true;
      }
      
      //Waits for user to action selfTest sequence
      else if (nextPressed == true){
        if(!printed){
          //Inital print to indicate user of current routine page
          Serial.println("Selftest");
          printed = true;
        }
        currentCase = 1;
      }

      //goint to Calibration
      if (previousPressed == true){
        if(!printed){
          //Inital print to indicate user of current routine page
          Serial.println("Calibration");
          printed = true;
        }        
        currentCase = 3;
      }


    break;



    case 1: // Self test Routine

      //Reading state of buttons
      actionPressed = readButtonDebounced(actionButton, actionStableState, actionLastReading, actionLastChangeTime);
      nextPressed = readButtonDebounced(nextButton, nextStableState, nextLastReading, nextLastChangeTime);
      previousPressed = readButtonDebounced(previousButton, previousStableState, previousLastReading, previousLastChangeTime);

      //Once this state is entered, user is prompted with text indicating this is the selftest Routine
      //We wait for teh user to press the action button to contine through the sequence of selfTest

      if(actionPressed == true){
        if(!printed){
          Serial.println("Place Device with arrow pointing to * and press action to continue");
          printed = true;
          subState = 1;
        }
      }
      //X axis
      if(subState == 1){
        axisWorking[0] = testInstance.selfTestData(0);
        if(!printed){
          Serial.println("Place Device with arrow pointing to * and press action to continue");
          printed = true;
          subState = 2;
        }
      }
      //Y axis
      else if(subState == 2){
        axisWorking[1] = testInstance.selfTestData(1);
        if(!printed){
          Serial.println("Place Device with arrow pointing to * and press action to continue");
          printed = true;
          subState = 2;
        }  
      }

      //Z axis
      else if(subState == 3){
        axisWorking[2] = testInstance.selfTestData(2);
        if(!printed){
          Serial.println("Place Device with arrow pointing to * and press action to continue");
          printed = true;
          subState = 4;
        }  
      }

      else if(subState == 4){
        if(!printed){
          Serial.println("results:");
          Serial.println(axisWorking[0]);
          Serial.println(axisWorking[1]);
          Serial.println(axisWorking[2]);
          Serial.println("press action to restart, or ");
          printed = true;
        }  

      }







    break;


    case 2: // Calibration



    break;


    case 3: // Step tracking


    break;

  }
}


