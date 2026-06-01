#pragma once
#include <Arduino.h>
#include <helper_math.h>
#include <adxl.h>


class selfTest{
public:
    selfTest(const adxl& accel) : accel(accel){}
    int selfTestData(int axis);
    bool selfTestComplete();
    void resetSelfTest();

    int currentAxis = 0;
private:
    const adxl& accel;
    const int stPin = 13;

    // float minChange[3] = {-0.800, -0.100, 0.200};
    // float maxChange[3] ={-0.200, 0.050, 1.330};
    float minChange[3] = {-0.799, 0.200, 0.200};
    float maxChange[3] = {-0.200, 0.799, 1.331};
    float initalVoltage;
};

