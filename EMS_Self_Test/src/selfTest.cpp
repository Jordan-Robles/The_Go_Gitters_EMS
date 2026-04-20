
#include <Arduino.h>
#include <math.h>
#include <accelerometer.h>

// Self test stuff
int selfTestCount[3] = {0}; // used to keep track of seft test pins
int selftTestState = 0; //used to iterate through each axis test state for now till i develop a function to handle whole sub-Routine
float Array[5] = {0};
int sampleIndex = 0;
bool selfTestRunning = false;
const unsigned long selfTestInterval = 1000; //gives us 3 seconds to read the axis value and determine if the sensor is working

//Time
unsigned long currentTime =0;
unsigned long previousTime =0;

//debugging
bool printed = false;


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