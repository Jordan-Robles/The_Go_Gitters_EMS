//step tracking subroutine

const float Step_Threshold = 0.39;
const int Window_Size = 5;
const unsigned long Min_Step_Interval = 400;

float accelBuffer[Window_Size];
int bufferIndex = 0;
bool bufferFilled = false;
unsigned long lastStepTime = 0;

void runStepTrack() {

  /*
   - check if a step has been detected
   - increase step counter
   - display relevent information on LCD screen
  */
    float magnitude = sqrt(accel.xAccel()*accel.xAccel() + accel.yAccel()*accel.yAccel() + accel.zAccel()*accel.zAccel());


    accelBuffer[bufferIndex] = magnitude;
    bufferIndex = (bufferIndex + 1) % Window_Size;
    if (bufferIndex == 0) bufferFilled = true;

    if (!bufferFilled) return false;

    float avg = 0;
    for (int i = 0; i < Window_Size; i++){
      avg += accelBuffer[i];
    }
    avg /= Window_Size;

    if ((magnitude - avg > Step_Threshold) && (millis() - lastStepTime > Min_Step_Interval)) {
      lastStepTime = millis();
      stepCounter.incrementSteps();
    }

    if (accel.yAccel() < 0.0001) {
    greenLED.set(false);
    redLED.set(false);
    return 0;  //idle no led
  }
 
  else if (accel.yAccel() >= 0.0001 && accel.yAccel() < 0.5){
    greenLED.set(false);
    redLED.set(true);
    return 1;
  }
 
  else if (accel.yAccel() >= 0.5 && accel.yAccel() < 0.55){
    greenLED.set(true);
    redLED.set(true);
    return 2;
  }
 
  else if (accel.yAccel() >= 0.55){
    greenLED.set(true);
    redLED.set(false);
    return 1;
  }
}

  void resetStepTrack() {
    stepCounter.resetStepCount();
    //reset the state of the mode
}
