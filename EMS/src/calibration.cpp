#include <Arduino.h>
#include <calibration.h>
#include <math.h>
#include <adxl.h>


static float Array[5] = {0};
static int sampleIndex = 0;
static const unsigned long selfTestInterval = 1000; //gives us 3 seconds to read the axis value and determine if the sensor is working
static unsigned long currentTime =0;
static unsigned long previousTime =0;

int offsetArray[3] = {0, 0, 0};

float oneG = 9.8;


int calibration::calibrationData(int axis){
  currentTime = millis();
  //------takes a sample every 1 second till a total of 5 samples is taken------
  if(currentTime - previousTime >= selfTestInterval && sampleIndex < 5){ 

    float reading = accel.read(axis);
    //we store the readings in an array so that it can be sent off to our averageArray function
    Array[sampleIndex] = reading;
    sampleIndex += 1;
    previousTime = currentTime;
    Serial.println(reading);
  }
  //------After smaples taken------
  else if(sampleIndex >= 5){
    float averageReading = averageArray(Array, 5);

    if(axis == 0){ //xAxis
      offsetArray[axis] = averageReading;
    }

    else if(axis == 1){
      offsetArray[axis] = averageReading;
    }

    else if(axis == 2){
      offsetArray[axis] = averageReading - oneG;
    }    
    sampleIndex = 0;

    // reset
    for(int i = 0; i < 5; i++)
    {
      Array[i] = 0;
    }

  }

  return offsetArray[axis];
}