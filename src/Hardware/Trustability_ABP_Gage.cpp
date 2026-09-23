/**
 * @file Trustability_ABP_Gage.cpp
 * @author Timothée Hirt & Paco Mermoud
 * @brief Class for a Trustability ABP gage pressure sensor
 * @version 0.1
 * @date 2022-01-29
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "Trustability_ABP_Gage.h"
#include <SPI.h>
#include <Arduino.h>
#include "C_output.h"
#include "Settings.h"


extern C_output output;

// See sensor datasheet
#define BAR_FACTORa 7.63e-4
#define BAR_FACTORb 1.25
#define TEMP_FACTOREa 9.77e-2
#define TEMP_FACTOREb 50

/**
 * @brief  Constructor for a pressure sensor
 *
 * @param _pin_slave_select Input connection on the board
 * @param _max_pressure Bar. Max pressure the sensor has to deal with
 */
void Trustability_ABP_Gage::begin(byte _pin_slave_select, float _max_pressure)
{
    pin_slave_select = _pin_slave_select;
    pinMode(pin_slave_select, OUTPUT);
    digitalWrite(pin_slave_select, HIGH);
    max_pressure = _max_pressure;
    output.println("Pressure sensor " + ID + " initiated");
}

/**
 * @brief  Constructor for a pressure sensor
 *
 * @param _pin_slave_select Input connection on the board
 * @param _max_pressure Bar. Max pressure the sensor has to deal with
 * @param _ID Name of sensor
 */
void Trustability_ABP_Gage::begin(byte _pin_slave_select, float _max_pressure, String _ID)
{
    ID = _ID;
    begin(_pin_slave_select, _max_pressure);
}

/**
 * @brief Pressure with reference 0 at atmospheric pressure
 *
 */
void Trustability_ABP_Gage::read()
{
    // SPI communication API
    SPI.beginTransaction(SPISettings(800000, MSBFIRST, SPI_MODE0));
    digitalWrite(pin_slave_select, LOW);
    byte byte1 = SPI.transfer(0); // control & pressure 14 bits
    byte byte2 = SPI.transfer(0); // pressure 14 bits
    byte byte3 = SPI.transfer(0); // temperature 8 bits
    digitalWrite(pin_slave_select, HIGH);
    SPI.endTransaction();

    // filter non valid data of pressure sensor. Two first bits should be 00, AND
    // not all three bytes zero - an all-zero frame passes the status-bit check
    // trivially but is what a disconnected/floating SPI line typically reads as,
    // not a real sensor response (a genuine reading is never exactly 0 counts).
    if (!(byte1 & B11000000) && !(byte1 == 0 && byte2 == 0 && byte3 == 0))
    {
        // output of pressure sensor in bits
        // concatenate data byte into one 16 byte then cast to float
        float output_pressure = (byte1 << 8) + byte2;
        float output_temp = (byte3 << 3) + B000;

        pressure = output_pressure * BAR_FACTORa - BAR_FACTORb;
        temperature = output_temp * TEMP_FACTOREa - TEMP_FACTOREb;
        if ((pressure > max_pressure) && PRESSURE_SENSOR_ERROR)
        {
            output.println("ERROR | Pressure to high on sensor " + ID + " (" + String(pressure) + " bar)");
        }
    }
    else
    {
        // this sensor gets polled very frequently (e.g. every loop iteration while
        // a pump is running) - printing on every single bad frame would flood the
        // terminal, so throttle it the same way as everything else that repeats.
        if (millis() - last_error_print > 2000)
        {
            output.println("ERROR | " + ID + ": invalid SPI response (bad status bits) - check wiring/CS");
            last_error_print = millis();
        }
    }
}

/**
 * @brief Return last pressure reading.
 *        Do not read faster than 1ms, see sensor datasheet.
 *        If so, data will be last correct pressure.
 *
 * @return float last pressure read.
 */
float Trustability_ABP_Gage::getPressure()
{
    // protection to not read to fast the sensor
    if (millis() - last_reading > 1)
    {
        last_reading = millis();
        read();
    }
    return pressure;
}

float Trustability_ABP_Gage::getTemperature()
{
    // protection to not read to fast the sensor
    if (millis() - last_reading > 1)
    {
        last_reading = millis();
        read();
    }
    return temperature;
}

float Trustability_ABP_Gage::getMaxPressure()
{
    return max_pressure;
}

String Trustability_ABP_Gage::getID()
{
    return ID;
}
