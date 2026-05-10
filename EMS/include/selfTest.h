#include <Arduino.h>
#include <math.h>
#include <adxl.h>

class selfTest{
public:
    selfTest(const adxl& accel) : accel(accel),ty_Voltage{-325.0, 325, 550} {}
    int selfTestData(int testCount);
private:
    const adxl& accel;
    const int STpin = 13;
    float [3];
};

