#ifndef PUMP_H
#define PUMP_H

#include <Arduino.h>
#include "MiniPID.h"
#include "Flow_sensor.h"
#include "Filter_watch.h"
#include "../src/Core/trusted_volume.h"
#include "../src/Core/sample_record.h"

extern MiniPID pump_pid;   // use this PID for pump, it is tuned for pressure control

// A plain "give me a pressure in bar" callback, not tied to any one sensor class -
// so the hard cap below can point at whichever sensor is actually trustworthy right
// now (pressure1/Trustability_ABP_Gage, ...) without Pump
// needing to know about every sensor type. Wire it up with a tiny free function,
// e.g. `float read_p_in() { return pressure1.getPressure(); }` in main.cpp.
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

    // Soft start (see move_output_to): the output as it is now (0-100 %), and whether a start is
    // ramping (the cap must then stop it too, though the pump is not running yet)
    uint8_t applied_percent = 0;
    bool ramping_ = false;
    void write_output(uint8_t percent);
    bool move_output_to(uint8_t target, bool always_ramp);

public:
    void begin(byte _control_pin, bool _pwm, byte _enable_pin = -1);
    void begin(byte _control_pin, bool _pwm, String _name, byte _enable_pin = -1);
    void set_flow(int _flow);
    void set_power(int8_t _power);
    // Live trim of a RUNNING pump: any fraction of a percent (12-bit DAC), never ramps, never blocks.
    // The caller owns the slew. 0 = pause (output 0, relay on); otherwise clamped to [PUMP_MINIMUM_POWER_PERCENT, 100].
    // The cap is still checked.
    void set_power_live(float _percent);
    uint8_t get_power();
    void start();
    // Start AT `_percent` at once: no 4 s soft start, so a regulation loop that begins right after is in control from
    // the first instant. Same refusals as start() (cap, O-ring locked). 1..9 become the 10 % floor. For test commands
    // that run their own PID loop; operation keeps start().
    void start_at(uint8_t _percent);
    void start(uint32_t _time_ms);
    void stop();
    bool is_running(){ return running; }

    // Registers a pressure-read callback + hard cap that set_power()/start() will
    // refuse to exceed, no matter which code path is calling them. Call once, in
    // setup(). Pass nullptr to disable the cap (e.g. if no sensor is trustworthy).
    void set_pressure_safety(PressureReadFn read_fn, float max_pressure_bar);
    // FW2: what is registered, so the sensorless test helper (src/Test_openloop.cpp, the
    // ONLY code that detaches the cap) can put back exactly what setup() registered.
    PressureReadFn get_pressure_safety_fn() { return safety_pressure_read_fn_; }
    float get_pressure_safety_cap() { return safety_max_pressure_; }
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
    // why the last run() stopped: "cond" (end condition met), "timeout", "cap", "sensor", "stalled", "clogged", "refused"
    const char *end_reason() const { return end_reason_; }
    // mean and peak pressure of the last run() (every plausible reading of its loop, pauses included; NaN if none)
    float pressure_mean() const { return pres_stats_.mean(); }
    float pressure_max() const { return pres_stats_.peak(); }
    // the flowmeter volume of the last run() counted at fast flow only (FLOW_TRUST_MIN_ML_PER_MIN, F150; 2026-10-23)
    float trusted_ml() const { return trusted_.trusted_ml(); }
protected:
    virtual bool check_end_cond(){ return false;}
    // true when the line is known to hold air now (only the emptying knows it): the power limit is then PUMP_AIR_MAX_POWER_PERCENT
    virtual bool air_now(){ return false; }
    // stop criteria of a filter run (stall, clogged, pressure); returns the reason to stop, or nullptr. Checked on every PID update.
    // paused = the pump is paused for the pressure limit now (see set_pause).
    virtual const char *check_watch(uint32_t now, float p, bool paused){ (void)now; (void)p; (void)paused; return nullptr; }
    const char *end_reason_ = "refused";
    core::PressureStats pres_stats_; // S1-C log: mean and peak of the pressure in the current run
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

    // Pause at the pressure limit (S1-C, 2026-10-19: the filter must be filled without overshooting the cap). When enabled,
    // the pump output goes to 0 at pause_bar_ and comes back, backed off, once the pressure is at resume_bar_ or below.
    bool pause_enabled_ = false;
    float pause_bar_ = 0;
    float resume_bar_ = 0;
    bool paused_ = false;
    float resume_power_ = 0;
    int n_pauses_ = 0;
    uint32_t pause_start_ = 0;
    uint32_t t_paused_ = 0;
    uint32_t ramp_t0_ = 0;   // F196: when the pump last started or restarted (the output rises from 0 since then)
    core::TrustedVolume trusted_; // the confident flowmeter volume of this run (F150, 2026-10-23)

    bool print_;
public:
    // turns the pause on for this run (filter runs only). Call before run().
    void set_pause(float pause_bar, float resume_bar){ pause_enabled_ = true; pause_bar_ = pause_bar; resume_bar_ = resume_bar; }
    int pauses() const { return n_pauses_; }
    uint32_t paused_ms() const { return t_paused_; }
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
    bool air_ = false;       // the last check: filtered pressure below the threshold = air
    bool air_now(){ return air_; }
};

class CtrlPumpFlow : public CtrlPump
{
public:
    // constructor or begin
    void begin(Pump *pump, MiniPID *pid, PressureReadFn pressure_read_fn,
               Flow_sensor *flow_sensor, float mL_thresh, float target_pressure = 2.5, bool print = false);
    void reset();
    // the stall / clogged rules of the filter run (the same ones as fill_test). Optional: without it the run only ends on volume, timeout or cap
    void set_filter_watch(FilterWatch *watch){ filter_watch_ = watch; }
protected:
    bool check_end_cond();
    const char *check_watch(uint32_t now, float p, bool paused);
    float milliL_tresh_;
    Flow_sensor* flow_sensor_;
    FilterWatch *filter_watch_ = nullptr;
};



#endif  // PUMP_H