#pragma once
#include <Arduino.h>
#include <adxl.h>


class stepCounter{
public:
    stepCounter(const adxl& accel) : accel(accel){}
    int runStepTrack();
    int numberOfSteps();
    void resetStepTrack();
    float maxMagnitude();
    void stepCounterV2(bool calibrated);
    void paceCalc(bool calibrated);
    int paceID();
    float distanceTravelled(float height);
    void sensitivityAdjustment(float height);

    
private:
    const adxl& accel; // Declare the member variable here

};

