#include <adxl.h>
#include <Arduino.h>
#include <selfTest.h>
#include <helper_math.h>


// Self test stuff
int st_axisWorking = 0; // used to keep track of seft test pins
float Array[5] = {0};
int sampleIndex = 0;
const unsigned long selfTestInterval = 500;
bool selfTestState = false;
float delta = 0.0;
bool testComplete = false;

// Time
unsigned long currentTime =0;
unsigned long previousTime =0;

int selfTest::selfTestData(int axis){
  currentTime = millis();
  if(selfTestState == false && !testComplete){ //if self test not started and not complete, start self test
    testComplete = false;
    initalVoltage = accel.readVoltage(axis);

    Serial.print("Pre-ST X: "); Serial.println(accel.readVoltage(0));
    Serial.print("Pre-ST Y: "); Serial.println(accel.readVoltage(1));
    Serial.print("Pre-ST Z: "); Serial.println(accel.readVoltage(2));

    digitalWrite(stPin, LOW); // 0V at gate, drain wil be 3.3v hence ST HIGH

    Serial.print("Post-ST X: "); Serial.println(accel.readVoltage(0));
    Serial.print("Post-ST Y: "); Serial.println(accel.readVoltage(1));
    Serial.print("Post-ST Z: "); Serial.println(accel.readVoltage(2));
    
    delay(100);
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

    delta = averageReading - initalVoltage;
    if(delta >= minChange[axis] && delta <= maxChange[axis]){// x in range of expected x value
      Serial.println(delta);
      Serial.println("axis Good");
      //We record the state of each axis in an array that will be used to display which axis isnt working well
      st_axisWorking = true;
      testComplete = true;
      selfTestState = false;
      digitalWrite(stPin, HIGH);
    }
    else{
      Serial.println(averageReading);
      Serial.println("axis no Good");
      st_axisWorking = false;
      testComplete = true;
      selfTestState = false;
      digitalWrite(stPin, HIGH);
    } 
  }
  return st_axisWorking;
}


bool selfTest::selfTestComplete(){
  return testComplete;
}

void selfTest::resetSelfTest(){
  testComplete = false;
  selfTestState = false;
  st_axisWorking = 0;
  digitalWrite(stPin, HIGH); // make sure ST is off
  delay(100);                // settle time between axes
}