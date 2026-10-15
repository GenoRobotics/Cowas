#ifndef CORE_PRESSURE_CALIBRATION_H
#define CORE_PRESSURE_CALIBRATION_H

// Pure math for the pressure sensor calibration against a reference manometer (S1-1,
// console `calibrate_pressure`): no Arduino.h, no globals, no hardware. Unit-tested on the
// host with `pio test -e native` (test/test_pressure_calibration/).
//
// Sensor and manometer share one closed air volume (a syringe on a tee). At each point the
// operator reads the manometer and the firmware averages the sensor's nominal (datasheet)
// reading. A straight line reference = gain x nominal + intercept is fitted over the points.

namespace core
{

// One calibration point.
struct PressureCalPoint
{
    float reference_bar; // read on the manometer
    float nominal_bar;   // the sensor's datasheet conversion, averaged
};

struct PressureCalFit
{
    bool valid;             // false: < 2 points, a bad input, no spread in the readings, or gain <= 0
    int n;                  // points used
    float gain;             // slope
    float intercept_bar;    // reference at nominal = 0
    float zero_offset_bar;  // nominal reading at reference = 0: -intercept / gain
    float r2;               // coefficient of determination (1 = every point on the line)
    float max_residual_bar; // largest |reference - (gain x nominal + intercept)|
};

PressureCalFit fit_pressure_cal(const PressureCalPoint *points, int n);

// reference - (gain x nominal + intercept) for one point. NaN if the fit is invalid.
float pressure_cal_residual(const PressureCalFit &fit, const PressureCalPoint &p);

enum PressureCalVerdict
{
    PRESSURE_CAL_PASS = 0,
    PRESSURE_CAL_FAIL_FIT,     // the fit is invalid (see PressureCalFit::valid)
    PRESSURE_CAL_FAIL_POINTS,  // fewer than min_points
    PRESSURE_CAL_FAIL_GAIN,    // gain outside [gain_min, gain_max]: wrong part number or wiring, not a calibration
    PRESSURE_CAL_FAIL_RESIDUAL // a point is further than max_residual_bar from the line
};

// Pass/fail of a calibration, checked in the order of the enum.
PressureCalVerdict pressure_cal_verdict(const PressureCalFit &fit, int min_points, float gain_min, float gain_max,
                                        float max_residual_bar);

// Short name of a verdict, for the TEST lines ("PASS", "FIT", "POINTS", "GAIN", "RESIDUAL").
const char *pressure_cal_verdict_name(PressureCalVerdict v);

} // namespace core

#endif
