#ifndef TRUSTABILITY_ABP_GAGE_H
#define TRUSTABILITY_ABP_GAGE_H

#include <Arduino.h>

#include <SPI.h>
#include "../src/Core/abp_convert.h"

class Trustability_ABP_Gage
{

private:
    int pin_slave_select;
    float pressure = 0;         // calibrated: PRESSURE_GAIN x (nominal - PRESSURE_ZERO_OFFSET_BAR)
    float nominal_pressure = 0; // datasheet conversion of the last valid frame
    float temperature = 0;
    String ID = "no_ID";
    float max_pressure = 3.2; // in bar; begin() sets it (main.cpp: STX_MAX_PRESSURE)
    uint32_t last_reading = 0;
    uint32_t last_error_print = 0; // throttles the "error pressure reading" print in read() - see there
    uint32_t last_high_print = 0;  // throttles the "Pressure to high" print in read() (F123)
    core::AbpTrust trust = core::abp_trust_init();
    core::AbpTrustState trust_state = core::ABP_UNTRUSTED_NEVER_VALID; // until the first valid frame
    core::AbpFrameStatus last_status = core::ABP_FRAME_STALE;
    void poll();

public:
    void begin(byte _pin_slave_select, float _max_pressure);
    void begin(byte _pin_slave_select, float _max_pressure, String _name);
    void read();
    void read_raw(byte frame[3]); // one SPI frame, unchecked (diagnostics: `abp_raw`)
    float getPressure();          // bar, calibrated; PRESSURE_SENSOR_INVALID_BAR when the sensor can't be trusted
    float getNominalPressure();   // bar, datasheet conversion only (for `pressure_zero` / `calibrate_pressure`); same invalid value
    bool isTrusted();             // result of the last read
    float getTemperature();
    float getMaxPressure();
    String getID();
};

void TC3_Handler();
#endif
