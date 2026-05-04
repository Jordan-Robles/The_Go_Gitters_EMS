#include <Arduino.h>
#include "accelerometer.h"

#include "accelerometer.h"

DFRobot_BMI160 bmi160;

bool accel_init(uint8_t i2c_addr) {
  bmi160.softReset();
  delay(100);

  if (bmi160.I2cInit(i2c_addr) != BMI160_OK) {
    Serial.println("BMI160 init failed!");
    return false;
  }

  Serial.println("BMI160 initialized");
  return true;
}