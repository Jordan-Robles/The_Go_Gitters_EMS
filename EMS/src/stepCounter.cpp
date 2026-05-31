#include "stepCounter.h" // <--- MUST INCLUDE THIS
#include <Arduino.h>
// #include <adxl.h> // (Usually not needed if included in stepCounter.h)

float Step_Threshold = 0.12; //we will be changing this so that the value can be changed 0.2 worked pre calibration
//0.15 works
const int Window_Size = 5;
unsigned long Min_Step_Interval = 400; //this will be changing so that the value can be changed, this is the minimum time between steps to avoid false positives
// 400 was gold
float accelBuffer[Window_Size];
int bufferIndex = 0;
bool bufferFilled = false;
unsigned long lastStepTime = 0;

unsigned long magCurrentTime = 0;
unsigned long magPreviousTime = 0;

float magnitude = 0.0;
float maxMag = 0.0;
int magArray[5];

int steps =0;

// === here is some stuff for Y axis measuring stepcounter ===
bool wasAboveThreshold = true;
unsigned long currentStepTime = 0;
unsigned longrunningInterval = 0;

unsigned long runningThreshold = 550;
unsigned long stationaryThreshold = 1000;
int pace = 0;
unsigned long interval = 0;


void incrementSteps(){
  steps++;
}


int stepCounter::numberOfSteps(){
  return steps;
}

// void stepCounter::stepCounterV2(bool calibrated) {
  
//   float rawY = accel.read(1);
//   Serial.println(rawY);
//   float threshold = 0.6; //0.7 works for Nate
//   if (rawY > threshold && !wasAboveThreshold && millis() - lastStepTime > Min_Step_Interval) {
//     if(calibrated){
//       incrementSteps();
//       paceCalc(true);
//       lastStepTime = millis();
//     }
//   }
//   wasAboveThreshold = (rawY > threshold);
// }

// int stepCounter::paceID(){
//   //Serial.println(pace);
//   if(steps == 0){
//     pace = 0;
//   }
//   else if(millis() - lastStepTime > stationaryThreshold){
//     pace = 0;
//   }
//   return pace;
// }

// void stepCounter::paceCalc(bool calibrated){
//   if(lastStepTime != 0){
//     unsigned long currentTime = millis();
//     unsigned long interval = currentTime - lastStepTime;

//     if (interval <= runningThreshold) {
//       // Running
//       Serial.println("Running");
//       pace = 2;
//     } else if (interval < stationaryThreshold) {
//       // Walking
//       Serial.println("Walking");
//       pace = 1;
//     } else {
//       // Stationary
//       Serial.println("Stationary");
//       pace = 0;
//     } 
//     } else {
//       Serial.println("walking");
//       pace = 1;
//   }
// }

float stepCounter::maxMagnitude(){
   magCurrentTime = millis();
  
  // Check if the 220ms window has passed
  if (magCurrentTime - magPreviousTime >= 2000) {
    // Reset for the next window
    magPreviousTime = magCurrentTime;
    maxMag = 0; // Reset max for the new window (or to a baseline value)
  }

  // Continuously track the max during the current window
  if (magnitude > maxMag) {
    maxMag = magnitude;
  }
  
  return maxMag;
}

// Changing to int since you return 0, 1, 2. (Make sure you change it in stepCounter.h too!)
int stepCounter::runStepTrack() { 
    // Assuming accel.xAccel() is meant instead of accel.read() for the X axis
    magnitude = sqrt(accel.read(0)*accel.read(0) + accel.read(1)*accel.read(1) + accel.read(2)*accel.read(2));
    //Serial.println(magnitude);
    accelBuffer[bufferIndex] = magnitude;
    bufferIndex = (bufferIndex + 1) % Window_Size;
    if (bufferIndex == 0) bufferFilled = true;

    if (!bufferFilled) return 0; // Changed false to 0 to match return type

    float avg = 0;
    for (int i = 0; i < Window_Size; i++){
      avg += accelBuffer[i];
    }
    avg /= Window_Size;

    if ((magnitude - avg > Step_Threshold) && (millis() - lastStepTime > Min_Step_Interval)) {
      lastStepTime = millis();
      incrementSteps(); // <--- REMOVED "stepCounter."
    }

    // if (accel.read(1) < 0.0001) {
    //   //greenLED.set(false); // Note: greenLED/redLED must be declared somewhere
    //   //redLED.set(false);
    //   return 0;  //idle no led
    // }
    // else if (accel.read(1) >= 0.0001 && accel.read(1) < 0.5){

    //   return 1;
    // }
    // else if (accel.read(1) >= 0.5 && accel.read(1) < 0.55){

    //   return 2;
    // }
    // else if (accel.read(1) >= 0.55){
    //   return 1;
    // }
    
    // return 0; // Fallback return
}

void stepCounter::resetStepTrack() { // <--- Added "stepCounter::" wrapper
    steps = 0; // Directly resets your global steps variable to 0
    bufferIndex = 0; // Good practice to reset your buffer tracking too
    bufferFilled = false;
}

void stepCounter::sensitivityAdjustment(float height){
  Step_Threshold = 0.12 * (height/1.7);
  Min_Step_Interval = 400 * (height/1.7);
}

float stepCounter::distanceTravelled(float height){
  
}
