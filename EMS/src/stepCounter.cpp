#include "stepCounter.h" // <--- MUST INCLUDE THIS
#include <Arduino.h>
// #include <adxl.h> // (Usually not needed if included in stepCounter.h)

const float Step_Threshold = 0.2;
const int Window_Size = 5;
const unsigned long Min_Step_Interval = 400;

float accelBuffer[Window_Size];
int bufferIndex = 0;
bool bufferFilled = false;
unsigned long lastStepTime = 0;

int steps =0;

void incrementSteps(){
    steps++;
}

int stepCounter::numberOfSteps(){
    return steps;
}

// Changing to int since you return 0, 1, 2. (Make sure you change it in stepCounter.h too!)
int stepCounter::runStepTrack() { 
    // Assuming accel.xAccel() is meant instead of accel.read() for the X axis
    float magnitude = sqrt(accel.read(0)*accel.read(0) + accel.read(1)*accel.read(1) + accel.read(2)*accel.read(2));

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

    if (accel.read(1) < 0.0001) {
      //greenLED.set(false); // Note: greenLED/redLED must be declared somewhere
      //redLED.set(false);
      return 0;  //idle no led
    }
    else if (accel.read(1) >= 0.0001 && accel.read(1) < 0.5){
;
      return 1;
    }
    else if (accel.read(1) >= 0.5 && accel.read(1) < 0.55){

      return 2;
    }
    else if (accel.read(1) >= 0.55){
      return 1;
    }
    
    return 0; // Fallback return
}

void stepCounter::resetStepTrack() { // <--- Added "stepCounter::" wrapper
    steps = 0; // Directly resets your global steps variable to 0
    bufferIndex = 0; // Good practice to reset your buffer tracking too
    bufferFilled = false;
}