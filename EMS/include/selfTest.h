#include <Arduino.h>
#include <math.h>
#include <adxl.h>

class selfTest{
public:
    selfTest(const adxl& accel) : accel(accel){}
    int selfTestData(int testCount);
private:
    const adxl& accel;
    const int STpin = 13;
};

