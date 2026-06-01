#pragma once
#include <Arduino.h>
#include <adxl.h>


class pacingID {
    public:
    pacingID(const adxl& accel) : accel(accel) {}
    int runPaceID();
    float getDynamicAccel() const { return _lastDynamicAccel; } // For debugging
    int paceTracker(int steps);
    float getMagnitude();
    private:
    const adxl& accel;

    static const int PIN_RED = 7;
    static const int PIN_YELLOW = 5;
    static const int PIN_GREEN = 6;

    static constexpr float THRESH_STATIONARY = 0.07f;
    static constexpr float THRESH_WALKING    = 0.3f;
    static constexpr float THRESH_RUNNING    = 1.0f;

    static constexpr unsigned long INTERVAL_MS = 250;
    

    //Accumalative state
    float _sum = 0.0f;
    int _count = 0;
    unsigned long _windowStart =0;
    
    int _currentPace = 0;
    unsigned long _lastActiveTime = 0; 
    static constexpr unsigned long ACTIVITY_MS = 1500;

    float _gravityBaseline = 1.0f; // Start with the assumption that we're at rest (1g) and adapt from there
    int _pendingPace = 0; // To hold the next pace to switch to after inactivity
    float _lastDynamicAccel = 0.0f; // For debugging

    //New paceID stuff
    static constexpr float STATIONARY_INTERVAL = 1000;
    static constexpr float WALKING_INTERVAL = 500;
    static constexpr float RUNNING_INTERVAL = 250;
    unsigned long _stepInterval = 0;
    unsigned long _startStep = 0;
    int _currentStep = 0;



};
