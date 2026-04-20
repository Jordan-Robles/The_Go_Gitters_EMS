/*
EMS Lab 2 Group 2

*/

#include <Arduino.h>
#include <DFRobot_BMI160.h>

//Creating accelerometer object and relvent variables
DFRobot_BMI160 bmi160;
const int8_t i2c_addr = 0x69;
// we declare this to be a size 6 as the frist 3 elements hold the gyro data while accel is the last 3
int16_t accel[6];

//Setting input pins for the ADXL to the Arduino nano
const int x = A0;
const int y = A1;
const int z = A2;
const int stPin = 13;

float xAxis = 0;
float yAxis = 0;
float zAxis = 0;

// Self test stuff
int selfTestCount[3] = {0}; // used to keep track of seft test pins
int selftTestState = 0; //used to iterate through each axis test state for now till i develop a function to handle whole sub-Routine
float Array[5] = {0};
int sampleIndex = 0;
bool selfTestRunning = false;
const unsigned long selfTestInterval = 1000; //gives us 3 seconds to read the axis value and determine if the sensor is working



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
}


bool serialButton(){
  if(Serial.available()){
    char c = Serial.read();
    return (c == 'w');
  }
  return false;
}


float averageArray(float array[], int size){
  //Helper function to take the average of recorded accelermoter values 
  float sum = 0.0;
  for(int i = 0; i <size; i++){
    sum = sum + array[i];
  }
  float average = abs(sum / size);

  return average;
}

int selfTestData(int testCount){
  currentTime = millis();
  //takes a sample every 1 second till a total of 5 samples is taken
  if(currentTime - previousTime >= selfTestInterval && sampleIndex < 5){ 
    //float reading = analogRead(axis); //uncommet for ADXL
    //bmi160.getAccelData(accel);
    bmi160.getAccelGyroData(accel);
    //float reading = accel[testCount] / 16384.0;
    float reading = accel[testCount]/16384.0;
    //we store the readings in an array so that it can be sent off to our averageArray function
    Array[sampleIndex] = reading;
    sampleIndex += 1;
    previousTime = currentTime;
    Serial.println(abs(reading));
  }
  else if(sampleIndex >= 5){
    float averageReading = averageArray(Array, 5);
    sampleIndex = 0;

    if(averageReading > 0.8 && averageReading < 1.2){// x in range of expected x value
      Serial.println("axis Good");
      //We record the state of each axis in an array that will be used to display which axis isnt working well
      selfTestCount[testCount-3] = 1; // testCount-3, it is minus 3 as we are taking account the fact that the x axis is indexed by 3 due to gyro 
    }
    else{
      Serial.println(averageReading);
      Serial.println("axis no Good");
      selfTestCount[testCount-3] = 0;
    } 
    selfTestRunning = false;
    selftTestState += 1;
    printed = false;
  }
}

void selfTest(int state ){

}




void loop() {
  switch(currentCase){
    case 0: // testing BMI160
    if(serialButton() == true){//Self test button pressed == HIGH
    Serial.println("Self Test initiated");
    Serial.print("X-Axis\t"); 
    Serial.println(selfTestCount[0]);
    Serial.print("Y-Axis\t"); 
    Serial.println(selfTestCount[1]);
    Serial.print("Z-Axis\t"); 
    Serial.println(selfTestCount[2]);

      previousTime = millis();
      currentCase = 1; // Test X 

    }
    break;

    case 1: // Self test Routine
    
      if(selftTestState == 0){ // xAxis
        if(!printed){
          Serial.println("Place Device with arrow pointing to * and press action button to continue");
          printed = true;
        }
        buttonPressed = serialButton();
        if(buttonPressed == true && selfTestRunning == false){ //replace with digitalRead(button3) ==
          printed = false;
          Serial.println("Starting xAxis");
          selfTestRunning = true;
        
        }
        else if(selfTestRunning == true){
          selfTestData(3);
        }
      }

      else if(selftTestState == 1){ // yAxis
        if(!printed){
          Serial.println("Place Device with arrow pointing to * and press action button to continue");
          printed = true;
        }
        buttonPressed = serialButton();
        if(buttonPressed == true && selfTestRunning == false){ //replace with digitalRead(button3) ==
          printed = false;
          Serial.println("Starting yAxis");
          selfTestRunning = true;
        
        }
        else if(selfTestRunning == true){
          selfTestData(4);
        }
      }

      else if(selftTestState == 2){ // zAxis
                if(!printed){
          Serial.println("Place Device with arrow pointing to * and press action button to continue");
          printed = true;
        }
        buttonPressed = serialButton();
        if(buttonPressed == true && selfTestRunning == false){ //replace with digitalRead(button3) ==
          printed = false;
          Serial.println("Starting zAxis");
          selfTestRunning = true;
        
        }
        else if(selfTestRunning == true){
          selfTestData(5);
        }
      }
      else if(selftTestState == 3){
        Serial.println("Self Test Complete, heading back to menu");
        currentCase = 0;
        selftTestState = 0;
      }

    break;
  }
}


