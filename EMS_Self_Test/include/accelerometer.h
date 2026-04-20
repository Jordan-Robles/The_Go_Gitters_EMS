
#include <Arduino.h>
#include <DFRobot_BMI160.h>

//Creating accelerometer object and relvent variables
DFRobot_BMI160 bmi160;
const int8_t i2c_addr = 0x69;
// we declare this to be a size 6 as the frist 3 elements hold the gyro data while accel is the last 3
int16_t accel[6];