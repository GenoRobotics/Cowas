#ifndef PRESSURE_SENSOR_H
#define PRESSURE_SENSOR_H

#include <Arduino.h>

//const uint8_t pressure_1_pin = A1;    // pressure 0-16Mpa (water and air)
//const uint8_t pressure_2_pin = A2;    // pressure 0-12 bar (cheaper one, only water)

class PressureSensor {
public:
    // Initialize the sensor
    void begin(int pin);

    // Read the pressure (analog voltage from input pin)
    float readPressure(); 
    virtual ~PressureSensor() = default;

protected:
    int _pin;
    float invertVoltageDivider(float voltage);
    // not the same for both sensors
    virtual float convertToPressure(float analogValue);
};


class  BigPressure : public  PressureSensor
{
private:
    float convertToPressure(float voltage);
};


class  SmallPressure : public  PressureSensor
{
private:
    float convertToPressure(float voltage);
};

#endif // PRESSURE_SENSOR_H