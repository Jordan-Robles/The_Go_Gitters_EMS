#include <Arduino.h>


class adxl{
public:

    adxl(): adxlPins{A0, A1, A2} {}
    //the read function will allow classes to call the reading from the adxl
    //where int 1, 2, 3 are ampped to x, y, z respectively 
    float read(int axis) const{ 
        float voltage = analogRead(adxlPins[axis]) * (5.0/1023.0);

        float reading = analogRead(adxlPins[axis]);
        return reading;
    }


private:
    const int adxlPins[3];

};

