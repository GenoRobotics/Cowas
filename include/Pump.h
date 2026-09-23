#ifndef PUMP_H
#define PUMP_H

#include <Arduino.h>
#include "MiniPID.h"
#include "Flow_sensor.h"

extern MiniPID pump_pid;   // use this PID for pump, it is tuned for pressure control

// A plain "give me a pressure in bar" callback, not tied to any one sensor class -
// so the hard cap below can point at whichever sensor is actually trustworthy right
// now (pressure1/Trustability_ABP_Gage, pressure2/BigPressure, ...) without Pump
// needing to know about every sensor type. Wire it up with a tiny free function,
// e.g. `float read_pressure2() { return pressure2.readPressure(); }` in main.cpp.
typedef float (*PressureReadFn)();

class Pump
{
private:
    byte control_pin;
    byte enable_pin;
    uint8_t power; // refers to power
    uint8_t power_percent;
    bool pwm; // if pump is controlled by pwm or only HIGH/LOW
    String ID = "no_ID";
    bool running = false;

    // Hard pressure-cap safety net - independent of whatever control loop (or manual
    // test) is driving this pump. See set_pressure_safety()/enforce_pressure_safety().
    PressureReadFn safety_pressure_read_fn_ = nullptr;
    float safety_max_pressure_ = 0;

public:
    void begin(byte _control_pin, bool _pwm, byte _enable_pin = -1);
    void begin(byte _control_pin, bool _pwm, String _name, byte _enable_pin = -1);
    void set_flow(int _flow);
    void set_power(int8_t _power);
    uint8_t get_power();
    void start();
    void start(uint32_t _time_ms);
    void stop();
    bool is_running(){ return running; }

    // Registers a pressure-read callback + hard cap that set_power()/start() will
    // refuse to exceed, no matter which code path is calling them. Call once, in
    // setup(). Pass nullptr to disable the cap (e.g. if no sensor is trustworthy).
    void set_pressure_safety(PressureReadFn read_fn, float max_pressure_bar);
    // Reads the registered sensor and force-stops the pump if it's over the cap.
    // Always does the sensor read when called (used internally by set_power()/
    // start() to decide whether to (re-)energize at all, even from a stopped
    // state). Callers outside the Pump class that just want a passive background
    // safety net for a pump that might already be running - e.g. a manual test
    // blocked waiting for a keypress - should guard the call with is_running()
    // first, so idle loops don't hit the sensor at all. Returns true if over cap.
    bool enforce_pressure_safety();
};

// --------------------- Classes to control the pump ------------------------------

// base class that makes sure the pressure doesn't get to high
class CtrlPump
{
public:
    void begin(Pump *pump, MiniPID *pid, PressureReadFn pressure_read_fn, float target_pressure = 2.5, bool print = false);
    void run();
    void set_max_pressure(float max_pressure){max_pressure_ = max_pressure;}
    void set_max_runtime(uint32_t max_runtime){max_runtime_ = max_runtime;}
    void set_end_cond_time(uint32_t time){time_end_valid_ = time;}
    void virtual reset(){;}
protected:
    virtual bool check_end_cond(){ return false;}
    Pump* pump_;
    MiniPID* pid_;
    PressureReadFn pres_read_fn_; // see PressureReadFn - whichever sensor is currently trustworthy, not tied to one type

    uint32_t start_time_;
    uint32_t max_runtime_;   // security timeout in case condition is never mets
    float target_pressure_;
    float max_pressure_;     // set to global variable

    uint32_t time_first_cond_met_;
    uint32_t time_end_valid_;        // time end condition must be valid
    bool end_cond_met_;  // if end condition already met at last iteration
    void reset_end_cond();
    bool update_end_cond(bool new_end_cond);     // true if should stop

    bool print_;
};


class CtrlPumpNoWater : public CtrlPump
{
public:
    // constructor or begin
    void begin(Pump *pump, MiniPID *pid, PressureReadFn pressure_read_fn, float target_pressure = 2.5, bool print = false);
    void set_pressure_thresh(float pres_tresh){pressure_thresh_ = pres_tresh;}   // if not the default
    void reset(){filtered_pressure_ = 0;}
protected:
    bool check_end_cond();
    float pressure_thresh_;  // below it is considered to be air
    float filtered_pressure_;
};

class CtrlPumpFlow : public CtrlPump
{
public:
    // constructor or begin
    void begin(Pump *pump, MiniPID *pid, PressureReadFn pressure_read_fn,
               Flow_sensor *flow_sensor, float mL_thresh, float target_pressure = 2.5, bool print = false);
    void reset();
protected:
    bool check_end_cond();
    float milliL_tresh_;
    Flow_sensor* flow_sensor_;
};



#endif  // PUMP_H