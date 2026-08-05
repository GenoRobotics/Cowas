#include <Arduino.h>
#include "Pump.h"
#include "Led.h"
#include "C_output.h"
#include "Settings.h"
#include "Flow_sensor.h"

MiniPID pump_pid = MiniPID(200, 100, 0);    // at 10Hz

extern C_output output;
extern Trustability_ABP_Gage pressure1;
extern Trustability_ABP_Gage pressure2;
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

    power = map(_power, 0, 100, 0, 255);
    power_percent = _power;
    // if pump already ON, then update the power live
    if (running)
        pump.start();
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
    // activating 24V Relay for power supply
    if (enable_pin != -1)
    {
        digitalWrite(enable_pin, HIGH);
    }

    if (pwm)
        analogWrite(control_pin, power);    // is actually DAC not PWM
    else
        digitalWrite(control_pin, HIGH);

    // display starting info only if previous state was OFF
    if (!running)
    {
        if(VERBOSE_PUMP){output.println("Pump " + ID + " started with power " + power_percent);}
        running = true;
    }
}

/**
 * @brief Start the pump with current power setting for a certain time, then stop
 *
 * @param time_ms The time is millisecond to run the pump.
 */
void Pump::start(uint32_t _time_ms)
{
    start();
    uint32_t current_time = millis();
    while (millis() - current_time < _time_ms)
    {
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
    if (pwm)
        analogWrite(control_pin, 0);
    else
        digitalWrite(control_pin, LOW);

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


void CtrlPump::begin(Pump *pump, MiniPID *pid, Trustability_ABP_Gage *pressure_sens, float target_pressure, bool print){
    pump_ = pump;
    pid_ = pid;
    pres_sens_ = pressure_sens;
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

    reset();

    // ! valves and motor need to be set before calling this function
    // run the pump
    pump_->set_power(50);
    pump_->start();

    pid_->setSetpoint(target_pressure_);

    start_time_ = millis();     // maybe no need to be class attribute

    float pressure_read;
    double PID_output;

    while (!update_end_cond(check_end_cond()))       // ? lower frequency of this with delay in loop?
    {
        if (millis() - last_update > 100){
            // control pump // ? maybe do a seperate fonction?
            pressure_read = pres_sens_->getPressure();
            PID_output = pid_->getOutput(pressure_read);
            pump_->set_power(PID_output);       // should be bound to 0-100
            last_update = millis();
            
        }
        if ((millis() - last_print > 1000) && print_){
            // print info
            // check with bool print
            Serial.print("Pressure bar : ");
            Serial.println(pres_sens_->getPressure());
            // Serial.print("Pump power : ");
            // Serial.println(pump_->get_power());
            Serial.print("Flow measured : ");
            Serial.println(flow_sensor_small.get_totalFlowMilliL());

            last_print = millis();
        }
        if (millis() - start_time_ > max_runtime_){
            Serial.println("loop break max runtime");
            break;
        }
        // if button should stop this, add condition here
    }

    pump_->stop();
    
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


void CtrlPumpNoWater::begin(Pump *pump, MiniPID *pid, Trustability_ABP_Gage *pressure_sens, float target_pressure, bool print){
    CtrlPump::begin(pump, pid, pressure_sens, target_pressure, print);
    pressure_thresh_ = EMPTY_WATER_PRESSURE_PURGE_THRESHOLD;
    filtered_pressure_ = 0;

    reset();
}

bool CtrlPumpNoWater::check_end_cond(){
    // using exponential filter to smooth peaks when there is only little water in pipes
    filtered_pressure_ = (1-0.3)*filtered_pressure_ + 0.3*pres_sens_->getPressure();
    return filtered_pressure_ < pressure_thresh_;
}


void CtrlPumpFlow::begin(Pump *pump, MiniPID *pid, Trustability_ABP_Gage *pressure_sens, 
               Flow_sensor *flow_sensor, float mL_thresh, float target_pressure, bool print){

    CtrlPump::begin(pump, pid, pressure_sens, target_pressure, print);
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