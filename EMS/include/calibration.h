#pragma once
#include <Arduino.h>
#include <helper_math.h>
#include <adxl.h>


class calibration{
public:
    calibration(const adxl& accel) : accel(accel) {}
    int calibrationData(int axis);
private:
    const adxl& accel;
    float sensitivity[3];
    float xOffset;
    float yOffset;
    float zOffset;
};

