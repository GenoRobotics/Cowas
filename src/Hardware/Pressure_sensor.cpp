#include "Pressure_sensor.h"
#include <Arduino.h>

//float invertVoltageDivider(float voltage);

// * Virtual function big
// Convert the analog voltage to pressure using a custom formula
float BigPressure::convertToPressure(float voltage) {

    const float  OffSet = 3.105 ; // V
    float pressure = (voltage - OffSet) * 250;   // these are kPa
    // Serial.print("Pressure kpa : ");
    // Serial.println(pressure);

    pressure = pressure / 100.0;    // to get bars
    // Serial.print("Pressure bar : ");
    // Serial.println(pressure);

    return pressure; // to get atm pressure
}

// * Virtual function SMALL
// Convert the analog voltage to pressure using a custom formula
float SmallPressure::convertToPressure(float voltage) {
    const float VCC = 4.75; // V
    float pressure = 15 * voltage / VCC - 1.5;

    return pressure;
}

// Initialize the sensor
void PressureSensor::begin(int pin) {
    _pin = pin;
    pinMode(_pin, INPUT);
}

// Read the pressure (analog voltage from input pin)
float PressureSensor::readPressure() {
    float voltage = analogRead(_pin);
    // Serial.print("Analog value : ");
    // Serial.println(voltage);
    voltage = voltage * 3.3 / 1023.0;
    // Serial.print("Voltage : ");
    // Serial.println(voltage);

    voltage = invertVoltageDivider(voltage);
    // Serial.print("Inverted voltage : ");
    // Serial.println(voltage);

    voltage = convertToPressure(voltage);
    // Serial.print("Pressure final return : ");
    // Serial.println(voltage);

    return voltage;
}



float PressureSensor::invertVoltageDivider(float voltage){
    // invert voltage divider on PCB
    // R1 = 1k, R2 = 1k+680Ohm
    return voltage * 1.68;   // * 1680.0 / 1000.0
}