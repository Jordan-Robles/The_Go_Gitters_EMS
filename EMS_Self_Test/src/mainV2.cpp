/*
EMS Lab 2 Group 2

*/

#include <Arduino.h>
#include <math.h>
#include <accelerometer.h>
#include <selfTest.h>
#include <stepCounting.h>


//Setting input pins for the ADXL to the Arduino nano
const int x = A0;
const int y = A1;
const int z = A2;
const int stPin = 13;

float xAxis = 0;
float yAxis = 0;
float zAxis = 0;

//Setting input pins for the buttons
const int button1 = 2; // Mode = toggles between each sub routine 
const int button2 = 3; // Next = insdie each subroutine we can flicker between different data to be displayed
const int button3 = 4; //Action button = COnfirms mode selection, start and stop

bool buttonPressed = false;
//Setting pins for LED
const int led1 = 7;
const int led2 = 6;
const int led3 = 5;

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

  Serial.begin(115200); //have it set as 112500 as of the moment to use with my existing BMI160 Accelerometer jsut to test code
   
  delay(100);

  bmi160.softReset();
  //bmi160.I2cInit(i2c_addr);
  if (bmi160.I2cInit(i2c_addr) != BMI160_OK) {
    Serial.println("BMI160 init failed!");
    while(1);
  }

  // Set pinMode for x, y, z pins as INPUT
  pinMode(x, INPUT);
  pinMode(y,INPUT);
  pinMode(z,INPUT);
  pinMode(stPin,OUTPUT);

  //Set buttons as INPUT
  pinMode(button1, INPUT);
  pinMode(button2,INPUT);
  pinMode(button3,INPUT);

  //Set State Led
  pinMode(led1, OUTPUT);
  pinMode(led2,OUTPUT);
  pinMode(led3,OUTPUT);


  // Setting ST pin as active low
  digitalWrite(stPin,HIGH);
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

    case 0: //menu
    break;

    case 1:// Self test Routine
    break;

    case 2:// Calibration Routine
     
      selfTestData(1);

    break;

    case 3://Step counting Routine
    break;  
  }  
}


