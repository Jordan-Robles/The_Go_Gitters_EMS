/*
EMS Lab 2 Group 2

*/

#include <Arduino.h>
#include <DFRobot_BMI160.h>
#include <adxl.h>
#include <selfTest.h>
#include <stepCounting.h>


//Setting input pins for the buttons
const int button1 = 2; // Mode = toggles between each sub routine 
const int button2 = 3; // Next = insdie each subroutine we can flicker between different data to be displayed
const int button3 = 4; //Action button = COnfirms mode selection, start and stop

bool buttonPressed = false;
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



void setup() {
  Serial.begin(9600); 

}


bool serialButton(){
  if(Serial.available()){
    char c = Serial.read();
    return (c == 'w');
  }
  return false;
}




void loop() {
  switch(currentCase){
    case 0: 

    break;

    case 1: // Self test Routine
      buttonPressed = serialButton(); 
      if(buttonPressed = true){
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


