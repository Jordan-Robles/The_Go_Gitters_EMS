#pragma once
#include <Arduino.h>
#include <adxl.h>

static const int CAL_NUM_SAMPLES  = 10;
static const unsigned long CAL_SAMPLE_INTERVAL_MS = 200; // 10 samples * 200ms = 2s total

class calibration {
public:
    // Note: non-const reference — calibration writes offsets back into adxl
    calibration(adxl& accel) : accel(accel) {}

    // Call repeatedly from loop(). Returns true once calibration is complete.
    // Device must be placed flat and stationary (gravity along +Z).
    // On completion, accel.read() will return values whose magnitude == 1g.
    bool calibrateAll();

    // True after calibrateAll() has finished at least once
    bool isCalibrated() const { return calibrated; }

    // Reset so calibrateAll() can be run again
    void reset();

private:
    adxl& accel;
    float samples[3][CAL_NUM_SAMPLES];
    int   sampleIndex  = 0;
    bool  calibrated   = false;

    unsigned long previousTime = 0;

    void computeAndApplyOffsets();
};
