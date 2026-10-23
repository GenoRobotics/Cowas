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

// See sensor datasheet. Pressure: src/Core/abp_convert (S1-1; the old 7.63e-4 / 1.25
// constants assumed a 10 bar part, F27).
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
void Trustability_ABP_Gage::read_raw(byte frame[3])
{
    // SPI communication API
    SPI.beginTransaction(SPISettings(ABP_SPI_HZ, MSBFIRST, SPI_MODE0));
    digitalWrite(pin_slave_select, LOW);
    frame[0] = SPI.transfer(0); // control & pressure 14 bits
    frame[1] = SPI.transfer(0); // pressure 14 bits
    frame[2] = SPI.transfer(0); // temperature 8 bits
    digitalWrite(pin_slave_select, HIGH);
    SPI.endTransaction();
}

void Trustability_ABP_Gage::read()
{
    byte frame[3];
    read_raw(frame);

    last_status = core::abp_classify_frame(frame[0], frame[1], frame[2]);
    if (last_status == core::ABP_FRAME_VALID)
    {
        nominal_pressure = core::abp_counts_to_bar(core::abp_frame_counts(frame[0], frame[1]), ABP_FULL_SCALE_PSI);
        pressure = core::abp_apply_calibration(nominal_pressure, PRESSURE_GAIN, PRESSURE_ZERO_OFFSET_BAR);
        float output_temp = (frame[2] << 3) + B000;
        temperature = output_temp * TEMP_FACTOREa - TEMP_FACTOREb;
        if ((pressure > max_pressure) && PRESSURE_SENSOR_ERROR && millis() - last_high_print > 2000)
        {
            // one line per 2 s: at 9600 baud 20 lines per second blocked the control loop for the whole second (F123)
            output.println("ERROR | Pressure to high on sensor " + ID + " (" + String(pressure) + " bar)");
            last_high_print = millis();
        }
    }

    // A stale frame (status 10) keeps the last value for a while; a fault frame, no answer,
    // or only stale frames for ABP_STALE_TIMEOUT_MS make the reading untrusted. getPressure()
    // then returns PRESSURE_SENSOR_INVALID_BAR, which the plausibility checks in Pump.cpp
    // turn into a pump stop: a dead or frozen sensor can no longer hide a rising pressure.
    trust_state = core::abp_trust_update(trust, last_status, millis(), ABP_STALE_TIMEOUT_MS);
    if (trust_state != core::ABP_TRUSTED)
    {
        // this sensor gets polled very frequently (e.g. every loop iteration while
        // a pump is running) - printing on every single bad frame would flood the
        // terminal, so throttle it the same way as everything else that repeats.
        if (millis() - last_error_print > 2000)
        {
            output.println("ERROR | " + ID + ": pressure reading not trusted (" + core::abp_trust_state_name(trust_state) +
                           ", last frame " + core::abp_frame_status_name(last_status) +
                           ") - check wiring/CS; the pump can't run");
            last_error_print = millis();
        }
    }
}

void Trustability_ABP_Gage::poll()
{
    // protection to not read to fast the sensor
    if (millis() - last_reading > 1)
    {
        last_reading = millis();
        read();
    }
}

/**
 * @brief Return last pressure reading (calibrated: gain and zero from Settings.h).
 *        Do not read faster than 1ms, see sensor datasheet.
 *        If so, data will be last correct pressure.
 *
 * @return float last pressure read, or PRESSURE_SENSOR_INVALID_BAR when the sensor
 *         can't be trusted (see read()).
 */
float Trustability_ABP_Gage::getPressure()
{
    poll();
    return (trust_state == core::ABP_TRUSTED) ? pressure : PRESSURE_SENSOR_INVALID_BAR;
}

/**
 * @brief Like getPressure(), without the gain and the zero offset: what the calibration
 *        commands average.
 */
float Trustability_ABP_Gage::getNominalPressure()
{
    poll();
    return (trust_state == core::ABP_TRUSTED) ? nominal_pressure : PRESSURE_SENSOR_INVALID_BAR;
}

bool Trustability_ABP_Gage::isTrusted()
{
    return trust_state == core::ABP_TRUSTED;
}

float Trustability_ABP_Gage::getTemperature()
{
    poll();
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
