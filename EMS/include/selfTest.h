#include <Arduino.h>
#include <math.h>
#include <adxl.h>


class selfTest{
public:
    selfTest(const adxl& accel) : accel(accel){}
    bool selfTestData(int axis);
private:
    const adxl& accel;
    const int stPin = 13;
    float minChange[3] = {-0.660, 0.165, 0.165};
    float maxChange[3] ={-0.165, 0.660, 1.100};
    float initalVoltage;
};

