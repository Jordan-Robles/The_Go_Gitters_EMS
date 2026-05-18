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
const int led1 = 2;
const int led2 = 3;
const int led3 = 4;

//Setting pins for the GC9A01
const int DIN = A4;
const int CLK = A5;
const int CS = 10;
const int DC = 9;

//Time
unsigned long currentTime =0;
unsigned long previousTime =0;

int currentCase = 0; // Set case = 0 to initialise system to starting profile

//debugging
bool printed = false;

//st pin stuff
int axisWorking[3];

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

    
    case 0: 

    break;

    case 1: // Self test Routine

      actionPressed = digitalRead(actionButton);
    
      if(actionPressed = true){
        if(!printed){
          Serial.println("Place Device with arrow pointing to * and press action button to continue");
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


