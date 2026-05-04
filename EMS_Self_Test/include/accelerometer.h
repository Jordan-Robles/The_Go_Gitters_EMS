#pragma once
#include <Arduino.h>
#include <DFRobot_BMI160.h>

// Expose the BMI160 object so other translation units can use it after init.
extern DFRobot_BMI160 bmi160;

// Initialize the sensor. Returns true on success.
bool accel_init(uint8_t i2c_addr = 0x69);