#include <Arduino.h>
#include <calibration.h>
#include <math.h>

// Physics:
//   Device flat on a surface → gravity is entirely in +Z.
//   Expected readings: X = 0g, Y = 0g, Z = 1g
//
//   Offsets applied to adxl:
//     offset_x = avg_x          (remove any bias from 0)
//     offset_y = avg_y          (remove any bias from 0)
//     offset_z = avg_z - 1.0f   (shift Z so it reads 1g not 1g+bias)
//
//   After correction: sqrt(read(0)^2 + read(1)^2 + read(2)^2) == 1.0


bool calibration::calibrateAll() {
    if (calibrated) return true;   // already done — nothing to do

    unsigned long now = millis();

    // Collect one simultaneous 3-axis sample every CAL_SAMPLE_INTERVAL_MS
    if ((now - previousTime >= CAL_SAMPLE_INTERVAL_MS) && (sampleIndex < CAL_NUM_SAMPLES)) {
        for (int axis = 0; axis < 3; axis++) {
            samples[axis][sampleIndex] = accel.read(axis);
        }
        sampleIndex++;
        previousTime = now;

        Serial.print("Cal sample ");
        Serial.print(sampleIndex);
        Serial.print("/");
        Serial.println(CAL_NUM_SAMPLES);
    }
    // Once all samples are in, compute and apply offsets
    if (sampleIndex >= CAL_NUM_SAMPLES) {
        computeAndApplyOffsets();
        return true;
    }

    return false;
}

void calibration::computeAndApplyOffsets() {
    float avg[3] = {0.0f, 0.0f, 0.0f};

    for (int axis = 0; axis < 3; axis++) {
        for (int i = 0; i < CAL_NUM_SAMPLES; i++) {
            avg[axis] += samples[axis][i];
        }
        avg[axis] /= CAL_NUM_SAMPLES;
    }

    // X and Y should read 0g when flat → offset = measured average
    // Z should read 1g when flat     → offset = measured average - 1.0
    accel.setOffset(0, avg[0]);
    accel.setOffset(1, avg[1] + 1.0f);
    accel.setOffset(2, avg[2]);
    //accel.setOffset(2, avg[2] - 1.0f);

    calibrated = true;

    // Debug: verify resulting magnitude is close to 1
    float mx = avg[0] - accel.getOffset(0);
    float my = avg[1] - accel.getOffset(1);
    float mz = avg[2] - accel.getOffset(2);
    float magnitude = sqrt(mx*mx + my*my + mz*mz);

    Serial.print("Calibration done. Offsets: X=");
    Serial.print(accel.getOffset(0)); Serial.print(" Y=");
    Serial.print(accel.getOffset(1)); Serial.print(" Z=");
    Serial.println(accel.getOffset(2));
    Serial.print("Corrected magnitude (expect 1.0): ");
    Serial.println(magnitude);
}




void calibration::reset() {
    sampleIndex = 0;
    calibrated  = false;
    previousTime = 0;
    for (int axis = 0; axis < 3; axis++)
        for (int i = 0; i < CAL_NUM_SAMPLES; i++)
            samples[axis][i] = 0.0f;
}