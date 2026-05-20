#pragma once
#include <Arduino.h>
#incude <adxl.h>


class pacingID {
    public:
    paceingID(const adxl& accel) : accel(accel) {}
    int runPaceID();

    private:
    const adxl& accel;

    static const int PIN_RED = 5;
    static const int PIN_YELLOW = 6;
    static const int PIN_GREEN = 7;

    static constexpr float THRESH_STATIONARY = 0.2f;
    static constexpr float THRESH_WALKING    = 1.5f;
    static constexpr float THRESH_RUNNING    = 4.0f;

    static constexpr unsigned long INTERVAL_MS = 250;

    //Accumalative state
    float _sum = 0.0f;
    int _count = 0;
    unsigned long _windowStart =0;
};
