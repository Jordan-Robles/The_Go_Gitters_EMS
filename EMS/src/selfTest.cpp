
#include <Arduino.h>
#include <selfTest.h>
#include <math.h>
#include <adxl.h>

// Self test stuff
int axisWorking = 0; // used to keep track of seft test pins
float Array[5] = {0};
int sampleIndex = 0;
const unsigned long selfTestInterval = 500;
bool selfTestState = false;

// Time
unsigned long currentTime =0;
unsigned long previousTime =0;

bool selfTest::selfTestData(int axis){
  currentTime = millis();
  if(selfTestState == false){
    initalVoltage = accel.readVoltage(axis);
    digitalWrite(stPin, LOW); // 0V at gate, drain wil be 3.3v hence ST HIGH
    delay(20);
    previousTime = millis();
    selfTestState = true;
  }
  //------takes a sample every 1 second till a total of 5 samples is taken------
  if(currentTime - previousTime >= selfTestInterval && sampleIndex < 5){ 

    float reading = accel.readVoltage(axis);
    //we store the readings in an array so that it can be sent off to our averageArray function
    Array[sampleIndex] = reading;
    sampleIndex += 1;
    previousTime = currentTime;
    Serial.println(reading);
  }
  //------After smaples taken------
  else if(sampleIndex >= 5){
    float averageReading = averageArray(Array, 5);
    sampleIndex = 0;

    if((averageReading - initalVoltage) >= minChange[axis] && (averageReading - initalVoltage) <= maxChange[axis]){// x in range of expected x value
      Serial.println("axis Good");
      //We record the state of each axis in an array that will be used to display which axis isnt working well
      axisWorking = true;
      selfTestState = false;
      digitalWrite(stPin, HIGH);
    }
    else{
      Serial.println(averageReading);
      Serial.println("axis no Good");
      axisWorking = false;
      selfTestState = false;
      digitalWrite(stPin, HIGH);
    } 
  }
  return axisWorking;
}