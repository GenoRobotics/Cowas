#include <Arduino.h>
#include "Pump.h"
#include "Led.h"
#include "C_output.h"
#include "Settings.h"
#include "Flow_sensor.h"
#include "Oring_lock.h"
#include "Run_keys.h"
#include "../Core/pump_ramp.h"
#include "../Core/pressure_ramp.h"
#include "../Core/pump_start.h"
#include "../Core/pump_power.h"

MiniPID pump_pid = MiniPID(200, 100, 0);    // at 10Hz

extern C_output output;
extern Led status_led;
extern Pump pump;
extern Flow_sensor flow_sensor_small;


/**
 * @brief Constructor for a diaphragm pump
 * @param _control_pin Output connection on the board
 * @param _pwm True if the control is PWM (analog pin). False if control is only HIGH/LOW (digital pin)
 *
 */
void Pump::begin(byte _control_pin, bool _pwm, byte _enable_pin)
{
    control_pin = _control_pin;
    pwm = _pwm;
    enable_pin = _enable_pin;
    pinMode(control_pin, OUTPUT);
    if (enable_pin != -1){
        pinMode(enable_pin, OUTPUT);
    }
    set_power(50);
    stop();
    output.println("Pump " + ID + " initiated");
}

void Pump::begin(byte _control_pin, bool _pwm, String _ID, byte _enable_pin)
{
    ID = _ID;
    begin(_control_pin, _pwm, _enable_pin);
}

/**
 * @brief Not implemented. Possiblity to control flow if power is mapped correspondingly.
 *
 * @param _flow ???
 */
void Pump::set_flow(int _flow)
{
    output.println("function -set_flow- not implemented");
}

/**
 * @brief Set pump power. Warning, on diaphragm pump, flow is not linear at all.
 *
 * @param _power From 0 to 100
 */
void Pump::set_power(int8_t _power)
{
    // data validation
    if (_power > 100)
        _power = 100;
    if (_power < 0)
        _power = 0;

    // pump deadband: below PUMP_MINIMUM_POWER_PERCENT the pump stalls and doesn't move smoothly.
    // clamp to minimum to prevent pulsing (on/off/on pattern) when PID tries low power to reduce overshoot
    if (_power > 0 && _power < PUMP_MINIMUM_POWER_PERCENT)
        _power = PUMP_MINIMUM_POWER_PERCENT;

    // hard pressure cap - refuse to (re-)energize if we're over the limit, regardless
    // of what the caller asked for
    if (enforce_pressure_safety())
    {
        _power = 0;
    }

    power = map(_power, 0, 100, 0, 255);
    power_percent = _power;
    // if pump already ON, then update the power live
    if (running)
        move_output_to(power_percent, false); // a live change: ramps when big, trims at once
}

void Pump::set_power_live(float _percent)
{
    if (!running || !pwm)
        return;
    if (_percent <= 0.0f)
        _percent = 0.0f; // a pause: output 0, the relay stays on, the next call above 0 resumes at once
    else if (_percent < PUMP_MINIMUM_POWER_PERCENT)
        _percent = PUMP_MINIMUM_POWER_PERCENT;
    if (_percent > 100.0f)
        _percent = 100.0f;
    if (enforce_pressure_safety()) // force-stops the pump itself
        return;
    power_percent = (uint8_t)(_percent + 0.5f);
    applied_percent = power_percent;
    analogWriteResolution(12); // the DAC is 12-bit; 8-bit steps are about 0.2 bar each on a clogged filter
    analogWrite(control_pin, (uint32_t)(_percent * 40.95f + 0.5f));
    analogWriteResolution(8);
}

/**
 * @brief Get current power set
 *
 * @return int power between 0 to 100.
 */
uint8_t Pump::get_power()
{
    return power_percent;
}

/**
 * @brief Start the pump with current power setting until stop command.
 *
 */
void Pump::start()
{
    // hard pressure cap - refuse to energize at all while over the limit
    if (enforce_pressure_safety())
    {
        return;
    }

    // target: the O-ring LOCKED before any pumping (an open O-ring leaks under pressure).
    // If it is open, oring_go_to() locks it first (a few seconds, the pump is still off).
    if (!oring_go_to(core::LOCK_STATE_LOCKED))
    {
        output.println("REFUSED | pump: O-ring not locked, pump not started");
        return;
    }

    // activating 24V Relay for power supply
    if (enable_pin != -1)
    {
        digitalWrite(enable_pin, HIGH);
    }

    // soft start: a start always ramps from 0 (move_output_to), so a surge is seen before it overshoots
    if (!running)
    {
        ramping_ = true;
        bool reached = move_output_to(power_percent, true);
        ramping_ = false;
        if (!reached)
        {
            return; // the cap stopped it during the ramp (SAFETY line printed by enforce_pressure_safety)
        }
        if(VERBOSE_PUMP){output.println("Pump " + ID + " started with power " + power_percent);}
        running = true;
    }
}

void Pump::start_at(uint8_t _percent)
{
    if (_percent > 100)
        _percent = 100;
    if (_percent > 0 && _percent < PUMP_MINIMUM_POWER_PERCENT)
        _percent = PUMP_MINIMUM_POWER_PERCENT;

    if (enforce_pressure_safety())
    {
        return;
    }
    if (!oring_go_to(core::LOCK_STATE_LOCKED))
    {
        output.println("REFUSED | pump: O-ring not locked, pump not started");
        return;
    }
    if (running)
    {
        set_power_live(_percent);
        return;
    }
    if (enable_pin != -1)
    {
        digitalWrite(enable_pin, HIGH);
    }
    power_percent = _percent;
    power = map(_percent, 0, 100, 0, 255);
    running = true;
    write_output(_percent);
    if(VERBOSE_PUMP){output.println("Pump " + ID + " started at once with power " + power_percent);}
}

/**
 * @brief Start the pump with current power setting for a certain time, then stop
 *
 * @param time_ms The time is millisecond to run the pump.
 */
void Pump::start(uint32_t _time_ms)
{
    if (run_abort_latched())
    {
        return; // x was typed earlier in this command: nothing more is pumped
    }
    run_keys_flush();
    start();
    uint32_t current_time = millis();
    while (millis() - current_time < _time_ms)
    {
        if (enforce_pressure_safety())
        {
            return; // already force-stopped inside enforce_pressure_safety()
        }
        // s skips this timed pump run, x aborts the command (S1-C addendum item 1)
        core::RunKey key = poll_run_key();
        if (key != core::RUN_KEY_NONE)
        {
            Serial.println(key == core::RUN_KEY_ABORT ? "ABORT | timed pump run stopped by x" : "SKIP | timed pump run skipped by s");
            break;
        }
        delay(5);
    }
    stop();
}

/**
 * @brief Stop the pump.
 *
 */
void Pump::stop()
{
    write_output(0);

    // deactivating 24V Relay for power supply
    if (enable_pin != -1)
    {
        // ! Do not uncomment, otherwise the pump still runs when resetting
        delay(500);
        digitalWrite(enable_pin, LOW);
    }

    running = false;
    if(VERBOSE_PUMP){output.println("Pump " + ID + " stopped");}
}

void Pump::write_output(uint8_t percent)
{
    applied_percent = percent;
    if (pwm)
        analogWrite(control_pin, map(percent, 0, 100, 0, 255)); // is actually DAC not PWM
    else
        digitalWrite(control_pin, percent > 0 ? HIGH : LOW);
}

// Moves the output to target. A start (always_ramp) and any big change ramp over PUMP_RAMP_MS in
// steps of PUMP_RAMP_STEP_MS; the cap is checked at every step. Returns false if the cap stopped the pump.
bool Pump::move_output_to(uint8_t target, bool always_ramp)
{
    if (!pwm)
    {
        write_output(target); // a digital pump can't be ramped
        return true;
    }
    const int from = applied_percent;
    if (!core::pump_ramp_needed(from, target, always_ramp, PUMP_RAMP_MIN_STEP_PERCENT))
    {
        write_output(target);
        return true;
    }
    const int steps = (int)(PUMP_RAMP_MS / PUMP_RAMP_STEP_MS);
    for (int i = 1; i <= steps; i++)
    {
        if (enforce_pressure_safety())
        {
            return false;
        }
        write_output((uint8_t)core::pump_ramp_percent(from, target, i, steps));
        delay(PUMP_RAMP_STEP_MS);
    }
    return true;
}

void Pump::set_pressure_safety(PressureReadFn read_fn, float max_pressure_bar)
{
    safety_pressure_read_fn_ = read_fn;
    safety_max_pressure_ = max_pressure_bar;
}

bool Pump::enforce_pressure_safety()
{
    if (safety_pressure_read_fn_ == nullptr)
    {
        return false; // no cap registered
    }

    float reading = safety_pressure_read_fn_();

    // Fail-safe on an implausible reading too, not just a high one - a reading this
    // far below 0 bar (gauge) means the sensor/wiring can't currently be trusted, so
    // treat it the same as "over the cap" rather than silently letting the pump run
    // unchecked. See PRESSURE_SENSOR_MIN_PLAUSIBLE_BAR (Settings.h) for why.
    bool over_cap = reading > safety_max_pressure_;
    bool implausible = reading < PRESSURE_SENSOR_MIN_PLAUSIBLE_BAR;

    if (over_cap || implausible)
    {
        if (running || ramping_)
        {
            output.println(String("SAFETY | ") + (implausible ? "implausible pressure reading (" : "pressure exceeded cap (") + String(reading) + " bar) - pump " + ID + " force-stopped");
            stop();
        }
        return true;
    }

    return false;
}


void CtrlPump::begin(Pump *pump, MiniPID *pid, PressureReadFn pressure_read_fn, float target_pressure, bool print){
    pump_ = pump;
    pid_ = pid;
    pres_read_fn_ = pressure_read_fn;
    target_pressure_ = target_pressure;
    print_ = print;

    max_pressure_ = STX_MAX_PRESSURE;   // can be adapted with the set function if needed
    max_runtime_ = (uint32_t)~0;    // largest number possible, need to change later
    time_end_valid_ = 0;
}


// ! add parameter runtime when calling function?
void CtrlPump::run(){
    uint32_t last_update = 0;
    uint32_t last_print = 0;

    end_cond_met_ = false;
    end_reason_ = "refused";
    paused_ = false;
    n_pauses_ = 0;
    t_paused_ = 0;
    pres_stats_.reset();

    reset();

    // x typed earlier in this command: this run does not start at all (the step above it stopped the procedure)
    if (run_abort_latched())
    {
        Serial.println("STOP | run aborted earlier in this command - pump not started");
        end_reason_ = "aborted";
        return;
    }
    run_keys_flush(); // a key typed before this run was for the step before it

    // ! valves and motor need to be set before calling this function
    // run the pump: AT ONCE at the start power, the PID in control from the first update. The old start was 50 %
    // open-loop through the 4 s soft start (F133): the PID was blind for 4 s and could overshoot meanwhile (user, 2026-10-15)
    // F196: from 0 %, rising at PUMP_START_RATE_PERCENT_PER_S (the PID output is capped by it below), not at once at 15 %
    float power_cmd = 0.0f;
    pump_->start_at(0);
    ramp_t0_ = millis();
    if (!pump_->is_running())
    {
        Serial.println("REFUSED | CtrlPump::run: the pump did not start (cap, sensor or O-ring)");
        return;
    }
    end_reason_ = "cond";

    pid_->setSetpoint(target_pressure_);

    start_time_ = millis();     // maybe no need to be class attribute
    // the confident flowmeter volume of this run starts from zero (2026-10-23, F150)
    trusted_.reset((float)flow_sensor_small.get_totalFlowMilliL(), start_time_);

    float pressure_read;
    double PID_output;

    while (!update_end_cond(check_end_cond()))       // ? lower frequency of this with delay in loop?
    {
        // every pass (not every 100 ms, F132): the cap, the sensor and the keys
        pressure_read = pres_read_fn_();
        // confident flowmeter volume: a 5 s window counts when its mean flow is at least FLOW_TRUST_MIN_ML_PER_MIN (F150)
        trusted_.update((float)flow_sensor_small.get_totalFlowMilliL(), millis(), FLOW_TRUST_MIN_ML_PER_MIN, FLOW_TRUST_WINDOW_MS);
        if (pressure_read >= PRESSURE_SENSOR_MIN_PLAUSIBLE_BAR){
            pres_stats_.add(pressure_read); // an implausible reading is not a pressure, so it is not counted
        }
        if (pressure_read > max_pressure_){
            Serial.println("SAFETY | max pressure (" + String(max_pressure_) + " bar) exceeded - stopping pump");
            end_reason_ = "cap";
            break;
        }
        // Fail-safe on an implausible (garbage) reading too - see
        // PRESSURE_SENSOR_MIN_PLAUSIBLE_BAR (Settings.h) / Pump::enforce_pressure_safety()
        // for why a wildly-negative reading must stop the pump, not be ignored.
        if (pressure_read < PRESSURE_SENSOR_MIN_PLAUSIBLE_BAR){
            Serial.println("SAFETY | implausible pressure reading (" + String(pressure_read) + " bar) - sensor not trustworthy, stopping pump");
            end_reason_ = "sensor";
            break;
        }
        // s skips this pump run (the step goes on), x aborts the command (S1-C addendum item 1)
        core::RunKey key = poll_run_key();
        if (key == core::RUN_KEY_SKIP){
            Serial.println("SKIP | pump run skipped by s - the step goes on");
            end_reason_ = "skipped";
            break;
        }
        if (key == core::RUN_KEY_ABORT){
            Serial.println("ABORT | run aborted by x - pump stopped, the command stops");
            end_reason_ = "aborted";
            break;
        }
        // pause at the pressure limit (fill_test rules, S1-C): the output goes to 0 and the relay stays on
        if (pause_enabled_ && !paused_ && pressure_read >= pause_bar_){
            paused_ = true;
            n_pauses_++;
            pause_start_ = millis();
            resume_power_ = power_cmd * FILL_BACKOFF_FACTOR;
            const float resume_limit = core::pump_power_limit_percent(air_now(), full_power_, PUMP_MAX_POWER_PERCENT, PUMP_AIR_MAX_POWER_PERCENT, PUMP_PURGE_MAX_POWER_PERCENT);
            if (resume_power_ > resume_limit){
                resume_power_ = resume_limit;
            }
            if (resume_power_ < (float)PUMP_MINIMUM_POWER_PERCENT){
                resume_power_ = (float)PUMP_MINIMUM_POWER_PERCENT;
            }
            pump_->stop(); // F196: the relay is cut, not only the output: at 0 % the pump still pushed and the pressure went on rising (P1)
            Serial.println("PAUSE at " + String(pressure_read, 2) + " bar (" + String(n_pauses_) + "): waiting for " + String(resume_bar_, 2) + " bar");
        }
        if (millis() - last_update > 100){
            // control pump // ? maybe do a seperate fonction?
            if (const char *why = check_watch(millis(), pressure_read, paused_)){
                Serial.println(String("STOP | filter ") + why + " - stopping pump");
                end_reason_ = why;
                break;
            }
            if (paused_){
                if (pressure_read <= resume_bar_){
                    paused_ = false;
                    t_paused_ += millis() - pause_start_;
                    pid_->reset(); // the integral from before the pause is no longer true
                    power_cmd = 0.0f; // F196: restart from 0 % and ramp up, as at the start
                    pump_->start_at(0);
                    ramp_t0_ = millis();
                    Serial.println("RESUME at " + String(pressure_read, 2) + " bar, restart from 0 % (ramp " + String(PUMP_START_RATE_PERCENT_PER_S, 1) + " %/s)");
                }
            }
            else{
                PID_output = pid_->getOutput(pressure_read);
                // water: 60 %, air: 100 % (Settings.h, user 2026-10-19); the purge: 100 % (exception, user 2026-10-26)
                const float power_limit = core::pump_power_limit_percent(air_now(), full_power_, PUMP_MAX_POWER_PERCENT, PUMP_AIR_MAX_POWER_PERCENT, PUMP_PURGE_MAX_POWER_PERCENT);
                if (PID_output > power_limit){
                    PID_output = power_limit;
                }
                // F196: after a start or a restart the output rises from 0 at a set rate
                const float ramp_limit = core::pump_start_limit(millis() - ramp_t0_, PUMP_START_RATE_PERCENT_PER_S);
                if (PID_output > ramp_limit){
                    PID_output = ramp_limit;
                }
                // at most PRESSURE_RAMP_MAX_STEP_PERCENT per update, and set_power_live never blocks: set_power made a
                // blocking 4 s ramp on steps of 10 points or more (F132)
                power_cmd = core::slew_limit(power_cmd, (float)PID_output, (float)PRESSURE_RAMP_MAX_STEP_PERCENT);
                pump_->set_power_live(power_cmd);
            }
            last_update = millis();

        }
        if ((millis() - last_print > 3000) && print_){ // throttled to avoid flooding the terminal during long pump runs
            // print info
            // check with bool print
            Serial.print("Pressure bar : ");
            Serial.println(pressure_read);
            // Serial.print("Pump power : ");
            // Serial.println(pump_->get_power());
            Serial.print("Flow measured : ");
            Serial.println(flow_sensor_small.get_totalFlowMilliL());

            last_print = millis();
        }
        if (millis() - start_time_ > max_runtime_){
            // the end rule did not fire in time: say so, with the last pressure, so the timeout is never silent (F155)
            Serial.println("loop break max runtime (" + String((millis() - start_time_) / 1000) + " s, last pressure " + String(pressure_read, 3) + " bar)");
            end_reason_ = "timeout";
            break;
        }
    }

    if (paused_){
        t_paused_ += millis() - pause_start_;
        paused_ = false;
    }
    trusted_.close((float)flow_sensor_small.get_totalFlowMilliL(), millis(), FLOW_TRUST_MIN_ML_PER_MIN); // the last, partial window
    pump_->stop();
    if (pause_enabled_){
        Serial.println("pauses=" + String(n_pauses_) + " paused_s=" + String(t_paused_ / 1000.0, 0));
    }

}


void CtrlPump::reset_end_cond(){
    end_cond_met_ = false;
}

// call this function from virtual function with condition checked
// this applies for checks that need to last over a certain time
// todo: add relaxation so that only one pressure spike is not enought to reset this condition
bool CtrlPump::update_end_cond(bool new_end_cond){
    bool ret_val = false;
    if (!new_end_cond && end_cond_met_){
        reset_end_cond();   // the end condition was not valid long enough so reset
        if (print_){Serial.println("Update end cond reset");}
    }
    else if (new_end_cond){
        if (!end_cond_met_){     // first time the end condition is met
            time_first_cond_met_ = millis();
            if(print_){Serial.println("End cond first met");}
        }
        else{   // when end condition valid long enough
            if (millis() - time_first_cond_met_ > time_end_valid_){
                ret_val = true;
                if(true){Serial.println("Pump control loop should stop now");}
            }
        }
    }

    end_cond_met_ = new_end_cond;
    return ret_val;
}


void CtrlPumpNoWater::begin(Pump *pump, MiniPID *pid, PressureReadFn pressure_read_fn, float target_pressure, bool print){
    CtrlPump::begin(pump, pid, pressure_read_fn, target_pressure, print);
    pressure_thresh_ = EMPTY_WATER_PRESSURE_PURGE_THRESHOLD;
    filtered_pressure_ = 0;

    reset();
}

bool CtrlPumpNoWater::check_end_cond(){
    // using exponential filter to smooth peaks when there is only little water in pipes
    filtered_pressure_ = (1-0.3)*filtered_pressure_ + 0.3*pres_read_fn_();
    air_ = filtered_pressure_ < pressure_thresh_;
    return air_;
}


void CtrlPumpFlow::begin(Pump *pump, MiniPID *pid, PressureReadFn pressure_read_fn,
               Flow_sensor *flow_sensor, float mL_thresh, float target_pressure, bool print){

    CtrlPump::begin(pump, pid, pressure_read_fn, target_pressure, print);
    flow_sensor_ = flow_sensor;
    milliL_tresh_ = mL_thresh;

    reset();    // somehow doesn't work well when doing start
}

bool CtrlPumpFlow::check_end_cond(){
    // update flow sensor reading
    // compare total flow to wanted flow
    flow_sensor_->update();
    return flow_sensor_->get_totalFlowMilliL() > milliL_tresh_;
}

void CtrlPumpFlow::reset(){
    flow_sensor_->reset_values();
    flow_sensor_->activate();
    Serial.println("Flow sensor reset, CtrlPumpFlow");
    Serial.print("Millil after reset: ");
    Serial.println(flow_sensor_->get_totalFlowMilliL());
    if (filter_watch_){
        filter_watch_->arm(millis(), 0.0f); // every run starts its stall windows at zero (run() calls reset() first)
    }
}

// Stall, clogged and pressure rules (src/Core/fill_stop.h, the same as fill_test). The pressure rule needs set_pause(): it
// stops the run when the pump has been paused and the pressure stays above the resume level for FILL_PRESSURE_STUCK_S.
const char *CtrlPumpFlow::check_watch(uint32_t now, float p, bool paused){
    if (!filter_watch_){
        return nullptr;
    }
    const core::FillStop r = filter_watch_->step(now, paused, (float)flow_sensor_->get_totalFlowMilliL(), p, resume_bar_);
    return r == core::FILL_GOING_ON ? nullptr : core::fill_stop_name(r);
}










/**
 * @brief Timer interrupt. Was implemented but now not used anymore.
 *
 */
void TC3_Handler()
{
    // You must do TC_GetStatus to "accept" interrupt
    // As parameters use the first two parameters used in startTimer (TC1, 0 in this case)
    TC_GetStatus(TC1, 0);

    // disable all text output
    ENABLE_OUTPUT = false;

    // TODO : Raise flag

    status_led.switch_state();

    // enable all text output back
    ENABLE_OUTPUT = true;
}

/**
 * @brief Timer interrupt. Was implemented but now not used anymore.
 *
 */
void TC4_Handler()
{
    // You must do TC_GetStatus to "accept" interrupt
    // As parameters use the first two parameters used in startTimer (TC1, 0 in this case)
    TC_GetStatus(TC1, 1);

    // disable all text output
    ENABLE_OUTPUT = false;

    // TODO : raise flag

    status_led.switch_state();

    // enable all text output back
    ENABLE_OUTPUT = true;
}