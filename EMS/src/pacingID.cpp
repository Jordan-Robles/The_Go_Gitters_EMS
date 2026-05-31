#include "pacingID.h"
//inherits arduino and adxl

//Pacing ID Variables:
// 0 = Stationary = Red LED --> less than 0.2 magnitude
// 1 = Walking = Yellow LED --> more than 0.2 less than 1.5
// 2 = Running = Green LED --> more than 1.5
// 3 is the secret state, all LEDs on 


// r5, y6, g7

int pacingID::runPaceID() {
    float x = accel.read(0);
    float y = accel.read(1);
    float z = accel.read(2);

    float magnitude = sqrt(x*x + y*y + z*z);

//summate the magnitudes over a period
    _sum += magnitude;
    _count += 1;
    
    //check over 250ms window
    unsigned long now = millis();
    if (now - _windowStart < INTERVAL_MS) {
        return -1; //just in the main write return if pace=-1
    }
    
    //window done,take average
    float avgMagnitude = 1.0f;
    if (_count > 0) {
        avgMagnitude = _sum/_count;
    }

    //reset window
    _sum = 0.0f;
    _count = 0;
    _windowStart = now;

    _gravityBaseline = (_gravityBaseline * 0.9f) + (avgMagnitude * 0.1f); 

    float dynamicAccel = abs(avgMagnitude - _gravityBaseline); // Subtract the adaptive gravity baseline to focus on movement, take absolute value to treat all movement as positive
    _lastDynamicAccel = dynamicAccel; // Store for debugging
    int pace;

    //Serial.println(avgMagnitude); 
    Serial.println(accel.read(1));
    
    if (dynamicAccel < THRESH_STATIONARY) {
        pace = 0; // Stationary
    } else if (dynamicAccel >= THRESH_STATIONARY && dynamicAccel < THRESH_WALKING) {
        pace = 1; // Walking
    } else if (dynamicAccel >= THRESH_WALKING && dynamicAccel < THRESH_RUNNING) {
        pace = 2; // Running
    } else {
        pace = 3; // Secret state - sprinting
    }

    //debouncing the magnitude readings so that we dont switch pace states too quickly
    if (pace > _currentPace) {
        _currentPace = pace; // Update the current pace immediately
        _pendingPace = pace; // Store the new pace as pending
        _lastActiveTime = millis(); // Reset the inactivity timer
    } 
    else if (pace != _pendingPace) {
        _pendingPace = pace; // If we're still in the same pace, keep it pending (or update it to the same value)
        _lastActiveTime = millis(); // Reset the inactivity timer even if the pace is the same, since we had movement
    }

    if(pace < _currentPace && (millis() - _lastActiveTime >= ACTIVITY_MS)) {
        _currentPace = _pendingPace; // Update to the pending pace after inactivity
    }

    if (_currentPace == 0) {
        digitalWrite(PIN_RED, HIGH);
        digitalWrite(PIN_YELLOW, LOW);
        digitalWrite(PIN_GREEN, LOW);
    } 
    else if (_currentPace == 1) {
        digitalWrite(PIN_RED, LOW);
        digitalWrite(PIN_YELLOW, HIGH);
        digitalWrite(PIN_GREEN, LOW);
    } 
    else if (_currentPace == 2) {
        digitalWrite(PIN_RED, LOW);
        digitalWrite(PIN_YELLOW, LOW);
        digitalWrite(PIN_GREEN, HIGH);
    } 
    else {
        digitalWrite(PIN_RED, HIGH);
        digitalWrite(PIN_YELLOW, HIGH);
        digitalWrite(PIN_GREEN, HIGH);
    }
 return _currentPace;
}

int pacingID::paceTracker(int steps) {
    unsigned long now = millis();
    if(_currentStep != steps){
        if(_currentStep != 0){
        
            _stepInterval = now - _startStep;
            //running
            if(_stepInterval <= 400){
                _currentPace = 2;
            } 
            //walking
            else if(_stepInterval <= 800 && _stepInterval > 400){
                _currentPace = 1;
                Serial.println("walking");
            }
            //stationary
            else if(_stepInterval > 800){
                _currentPace = 0;
            }
        }
        _startStep = now;
        _currentStep = steps;
    }
    if((now - _startStep) > 1200){
        _currentPace = 0;
    }
    if (_currentPace == 0) {
        digitalWrite(PIN_RED, HIGH);
        digitalWrite(PIN_YELLOW, LOW);
        digitalWrite(PIN_GREEN, LOW);
    } 

    else if (_currentPace == 1) {
        digitalWrite(PIN_RED, LOW);
        digitalWrite(PIN_YELLOW, HIGH);
        digitalWrite(PIN_GREEN, LOW);
    } 

    else if (_currentPace == 2) {
        digitalWrite(PIN_RED, LOW);
        digitalWrite(PIN_YELLOW, LOW);
        digitalWrite(PIN_GREEN, HIGH);
    } 

    else {
        digitalWrite(PIN_RED, LOW);
        digitalWrite(PIN_YELLOW, LOW);
        digitalWrite(PIN_GREEN, LOW);
    }
    return _currentPace;
}