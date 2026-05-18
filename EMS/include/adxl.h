#include <Arduino.h>


class adxl{
public:

    adxl(): adxlPins{A0, A1, A2}, voltage_zero{1.5, 1.5, 1.5} {}
    
    //the read function will allow classes to call the reading from the adxl
    //where int 1, 2, 3 are ampped to x, y, z respectively 
    float read(int axis) const{ 
        // converts the analog signal to usable volatge value
        float voltage = analogRead(adxlPins[axis]) * (5.0/1023.0);
        //convert the voltage to acceleration, the 0.3V (300mV comes from the senesitivty from adxl datasheet)
        float reading = (voltage - voltage_zero[axis])/ 0.3;
        return reading;
    }

    float readVoltage(int axis) const{
        float voltage = analogRead(adxlPins[axis]) * (5.0/1023.0);
        return voltage;
    }

protected:
    //this will hold the 0g volatage for each pin, the values will be obtained via calbration
    float voltage_zero[3];


private:
    const int adxlPins[3];

};

