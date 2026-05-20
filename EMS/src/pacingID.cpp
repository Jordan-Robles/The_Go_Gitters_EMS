#include "pacingID.h"
//inherits arduino and adxl

//Pacing ID Variables:
// 0 = Stationary = Red LED --> less than 0.2 magnitude
// 1 = Walking = Yellow LED --> more than 0.2 less than 1.5
// 2 = Running = Green LED --> more than 1.5
// 3 is the secret state, all LEDs on 


// r5, y6, g7

int pacingID::runPaceID() {
    float x = accel.getX();
    float y = accel.getY();
    float z = accel.getZ();

    float magnitude = sqrt(x*x + y*y + z*z);
//summate the magnitudes over a period
    _sum += magnitude;
    _count += 1;
    
    //check over 250ms window
    unsigned long now = millis();
    if (now - _windowStart >= INTERVAL_MS) {
        return -1; //just in the main write return if pace=-1
    }
    
    //window done,take average
    if (_count > 0) {
    float avgMagnitude = _sum/_count;
    }

    //reset window
    _sum = 0.0f;
    _count = 0;
    _windowStart = now;

    int pace;

    if (magnitude < THRESH_STATIONARY) {
        pace = 0; // Stationary
        digitalWrite(PIN_RED, HIGH);
        digitalWrite(PIN_YELLOW, LOW);
        digitalWrite(PIN_GREEN, LOW);
    } else if (magnitude >= THRESH_STATIONARY && magnitude < THRESH_WALKING) {
        pace = 1; // Walking
        digitalWrite(PIN_RED, LOW);
        digitalWrite(PIN_YELLOW, HIGH);
        digitalWrite(PIN_GREEN, LOW);
    } else if (magnitude >= THRESH_WALKING && magnitude < THRESH_RUNNING) {
        pace = 2; // Running
        digitalWrite(PIN_RED, LOW);
        digitalWrite(PIN_YELLOW, LOW);
        digitalWrite(PIN_GREEN, HIGH);
    } else {
        pace = 3; // Secret state - sprinting
        digitalWrite(PIN_RED, HIGH);
        digitalWrite(PIN_YELLOW, HIGH);
        digitalWrite(PIN_GREEN, HIGH);
    }

 return pace;
}
