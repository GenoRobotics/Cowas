#include <Arduino.h>
#include "Motor.h"
#include "DualVNH5019MotorShield.h"
#include "Encoder.h"
#include "Settings.h"
#include "Button.h"
#include "Critical_error.h"
#include "C_output.h"
#include "Run_keys.h"
#include "Core/spool_motion.h"
#include <math.h>

extern C_output output;

unsigned char INA1 = MOTOR_INA1_PIN;         // INPUT - motor 1 direction input A
unsigned char INB1 = MOTOR_INB1_PIN;         // INPUT - motor 1 direction input B
unsigned char PWM1 = MOTOR_PWM1_PIN;         // INPUT - motor 1 speed input
unsigned char EN1DIAG1 = MOTOR_EN1DIAG1_PIN; // Non attributed OUTPUT - motor 1 enable input/fault output
unsigned char CS1 = MOTOR_CS1_PIN;           // Non attributed// OUTPUT - motor 1 current sense
unsigned char INA2 = MOTOR_INA2_PIN;         // Non attributed// INPUT - motor 2 direction input A
unsigned char INB2 = MOTOR_INB2_PIN;         // Non attributed// INPUT - motor 2 direction input B
unsigned char PWM2 = MOTOR_PWM2_PIN;          // Non attributed// INPUT - motor 2 speed input
unsigned char EN2DIAG2 = MOTOR_EN2DIAG2_PIN; // Non attributed// OUTPUT - motor 2 enable input/fault output
unsigned char CS2 = MOTOR_CS2_PIN;           // Non attributed// OUTPUT - motor 2 current sense

// motor initialization with polulu driver

DualVNH5019MotorShield md(INA1, INB1, PWM1, EN1DIAG1, CS1, INA2, INB2, PWM2, EN2DIAG2, CS2);
extern Encoder encoder;
extern Motor spool;
extern Button button_spool_up;
extern Button button_spool_down;

/**
 * @brief Constructor for a motor with polulu driver
 *
 */
void Motor::begin()
{
    md.init();
    if (VERBOSE_INIT){output.println("Motor " + ID + " initiated");}
    
}

/**
 * @brief Constructor for a motor with polulu driver
 *
 */
void Motor::begin(String _ID)
{
    ID=_ID;
    begin();
}

/**
 * @brief set speed
 *
 * @param _speed Between 0 and 100. Always positive.
 * @param _direction Up or down.
 */
void Motor::set_speed(uint8_t _speed, motor_direction _direction)
{
    if (_speed > 100)
        _speed = 100;
    if (_speed < 0)
        _speed = 0;
    speed = map(_speed, 0, 100, 0, 400);
    direction = _direction;
}

/**
 * @brief Start the motor with the current settings of speed and direction
 *
 */
void Motor::start()
{
     if (ID=="MANIFOLD")
        {
            if (VERBOSE_MOTOR){output.println("Start motor manifold");}
            // md.setM2Speed(speed * direction == up ? 1 : -1);
            float a;
            if (direction==up){
                md.setM2Speed(speed);
                if (VERBOSE_MOTOR){output.println("ENTER");}
                a=speed;
            }
            else{
                md.setM2Speed(-speed);
                if (VERBOSE_MOTOR){output.println("ENTER");}
                a=-speed;
            }
            if (VERBOSE_MOTOR){output.println("Speed : "+String(speed)+ " , direction : "+ String(direction)+ " , Resultat : "+String(a));}
        }
    else{
        // Do not start if already at the very top or very down and order to go further
        if ((direction == up && endstop_up) || (direction == down && endstop_down))
        {
            if (VERBOSE_MOTOR){output.println("Reach end of tube, order cancelled | cannot wind up more");}
            
        }
        else
        {
            endstop_up = false;
            endstop_down = false;
            // set here direction of motor
            md.setM1Speed(speed * (direction == up ? 1 : -1));
            if (VERBOSE_MOTOR){output.println("Motor started with speed " + String(speed) + " in direction " + (direction == up ? "up" : "down"));}
        }
    }
}

/**
 * @brief Start the motor with a given speed and direction
 *
 * @param _speed Between 0 and 100. Always positive.
 * @param _direction Up or down.
 */
void Motor::start(uint8_t _speed, motor_direction _direction)
{
    set_speed(_speed, _direction);
    start();
}

/**
 * @brief Legacy entry (the `spool` test commands): go to a depth in cm below the water, 0 or less = back to the origin.
 * Goes through move_to_length()/go_home(): stall, wrong-way and time limits, and the length limit SPOOL_MAX_LENGTH_CM.
 *
 * @param _depth in centimeters. Zero or negative: roll up to the origin.
 */
void Motor::start(int _depth)
{
    const char *why = "";
    const float length = _depth <= 0 ? 0.0f : (float)_depth + spool_height_cm;
    const bool ok = length <= 0.5f ? go_home(&why) : move_to_length(length, &why);
    if (!ok)
    {
        output.println(String("ERROR | spool: move to ") + String(_depth) + " cm failed: " + why);
    }
}

float Motor::length_cm()
{
    return encoder.get_distance_f();
}

// Sets the VNH5019 M1 speed without start(): start() clears the end-stop flags, which would hide a switch hit during the move.
static void spool_apply_speed(Motor &m, uint8_t pct, motor_direction dir)
{
    m.set_speed(pct, dir);
    const int duty = map(pct, 0, 100, 0, 400);
    md.setM1Speed(dir == up ? duty : -duty);
}

/**
 * @brief One spool move, blocking. Goes `dir` until the goal length (or the origin switch when to_switch) and stops on the
 * first of: arrival, origin switch, down end-stop, no encoder pulse for SPOOL_STALL_MS, the length going the wrong way,
 * SPOOL_MOVE_MAX_MS, x typed. Keeps reading the encoder for SPOOL_COAST_COUNT_MS after the brake so the length stays true.
 * Nothing is printed inside the loop (the Due must keep polling the encoder).
 *
 * @return true if the goal (or the switch) was reached. Otherwise *why says what stopped it.
 */
bool Motor::drive(motor_direction dir, uint8_t fast_speed, float goal_cm, bool to_switch, const char **why)
{
    *why = "";
    run_keys_flush(); // a key typed before this move must not stop it
    if (run_abort_latched())
    {
        *why = "aborted with x";
        return false;
    }
    endstop_up = false;
    endstop_down = false;
    if (dir == up && button_spool_up.getState() == 0)
    {
        if (homed && encoder.get_distance_f() > 3.0f * SPOOL_SLOW_ZONE_CM)
        {
            // the encoder says the tube is well out but the switch says it is at the top: wrong wiring level or lost position.
            // Do not "arrive" without moving: the tube would stay out and the next dive would start from a false zero.
            *why = "origin switch reads pressed but the encoder says the tube is out: check the switch wiring (spool_status)";
            return false;
        }
        // already on the origin switch: the ISR only fires on a new press
        encoder.reset();
        homed = true;
        if (to_switch)
        {
            return true;
        }
        *why = "origin switch already pressed: the position was wrong, re-homed";
        return false;
    }
    if (dir == down && button_spool_down.getState() == 0)
    {
        *why = "down end-stop pressed";
        return false;
    }

    const float start_cm = encoder.get_distance_f();
    core::SpoolMoveParams p = {dir == down ? +1 : -1, to_switch, goal_cm, SPOOL_MOVE_MAX_MS, SPOOL_STALL_MS, SPOOL_WRONG_WAY_CM};
    // speed: unknown position = slow all the way to the switch; known = fast, slow near the goal or the origin
    auto speed_for = [&](float now_cm) -> uint8_t {
        if (to_switch)
        {
            return homed ? core::spool_approach_speed(now_cm, SPOOL_SLOW_ZONE_CM, fast_speed, SPOOL_HOME_SPEED) : SPOOL_HOME_SPEED;
        }
        return core::spool_approach_speed(fabsf(goal_cm - now_cm), SPOOL_SLOW_ZONE_CM, fast_speed, SPOOL_SLOW_SPEED);
    };
    uint8_t speed_now = speed_for(start_cm);
    set_speed(speed_now, dir);
    start();

    int32_t last_pulses = encoder.get_pulses_A();
    const uint32_t t0 = millis();
    uint32_t t_pulse = t0, t_tick = t0, t_key = t0;
    bool arrived = false, at_switch = false;
    for (;;)
    {
        encoder.step_counter();
        const uint32_t now = millis();
        if (now == t_tick)
        {
            continue; // everything below once per millisecond
        }
        t_tick = now;
        const int32_t pulses = encoder.get_pulses_A();
        if (pulses != last_pulses)
        {
            last_pulses = pulses;
            t_pulse = now;
        }
        if (dir == up)
        {
            if (endstop_up || button_spool_up.getState() == 0)
            {
                at_switch = true;
                break;
            }
        }
        else if (button_spool_down.getState() == 0)
        {
            *why = "down end-stop pressed";
            break;
        }
        const float cur = encoder.get_distance_f();
        const core::SpoolMove state = core::spool_move_state(p, start_cm, cur, now - t0, now - t_pulse);
        if (state == core::SPOOL_MOVE_ARRIVED)
        {
            arrived = true;
            break;
        }
        if (state != core::SPOOL_MOVE_GO)
        {
            *why = core::spool_move_name(state);
            if (state == core::SPOOL_MOVE_STALLED || state == core::SPOOL_MOVE_WRONG_WAY)
            {
                homed = false; // the encoder cannot be trusted any more
            }
            break;
        }
        if (now - t_key >= 50)
        {
            t_key = now;
            poll_run_key();
            if (run_abort_latched())
            {
                *why = "aborted with x";
                break;
            }
        }
        const uint8_t wanted = speed_for(cur);
        if (wanted != speed_now)
        {
            speed_now = wanted;
            spool_apply_speed(*this, speed_now, dir);
        }
    }
    stop();
    const uint32_t t_coast = millis();
    while (millis() - t_coast < SPOOL_COAST_COUNT_MS)
    {
        encoder.step_counter();
    }
    if (at_switch)
    {
        encoder.reset(); // the origin is the switch
        homed = true;
        if (to_switch)
        {
            return true;
        }
        *why = "origin switch reached before the goal: the position was wrong, re-homed";
        return false;
    }
    return arrived;
}

/**
 * @brief Move to a length (cm of tube out, from the origin switch). The position is unknown after every boot: the first
 * move rolls up to the origin switch before anything else.
 */
bool Motor::move_to_length(float length_cm_goal, const char **why)
{
    *why = "";
    if (!SPOOL_USE)
    {
        *why = "SPOOL_USE is false";
        return false;
    }
    if (!(length_cm_goal >= 0.0f) || length_cm_goal > SPOOL_MAX_LENGTH_CM)
    {
        *why = "length outside 0..SPOOL_MAX_LENGTH_CM";
        return false;
    }
    if (!homed)
    {
        output.println("Spool: position unknown (reset): rolling up to the origin switch first");
        if (!go_home(why))
        {
            return false;
        }
    }
    const float now = encoder.get_distance_f();
    if (fabsf(length_cm_goal - now) < 1.0f)
    {
        return true;
    }
    if (length_cm_goal <= 0.5f)
    {
        return go_home(why);
    }
    return drive(length_cm_goal > now ? down : up, length_cm_goal > now ? SPEED_DOWN : SPEED_UP, length_cm_goal, false, why);
}

bool Motor::go_home(const char **why)
{
    *why = "";
    if (!SPOOL_USE)
    {
        *why = "SPOOL_USE is false";
        return false;
    }
    return drive(up, SPEED_UP, 0.0f, true, why);
}

/**
 * @brief Roll up to the origin switch (legacy name, used by maintenance.cpp).
 */
void Motor::start_origin()
{
    const char *why = "";
    if (!go_home(&why))
    {
        output.println(String("ERROR | spool: roll up to the origin failed: ") + why);
    }
}

/**
 * @brief Stop the motor with a fast deceleration.
 *
 */
void Motor::stop()
{
    if(ID=="MANIFOLD"){
        md.setM2Brake(400);
    }
    else{
        md.setM1Brake(300);}
    if (VERBOSE_MOTOR){output.println("Motor stopped");}
}

/**
 * @brief Not used. Stop the motor if driver return an error.
 *
 */
void Motor::stopIfFault()
{
    if (md.getM1Fault())
    {
        output.println("M1 fault");
        while (1);
    }
}

/**
 * @brief Interrupt function with spool microswitches
 * Do not serial print here, cause crash
 *
 */
void ISR_emergency_stop_up()
{
    if (spool.direction == up && spool.endstop_up == false)
    {
        spool.endstop_up = true;
        md.setM1Brake(400);
        encoder.reset();
        if (VERBOSE_MOTOR){output.println("Endstop up touched");}
    }
}

/**
 * @brief Interrupt function
 * Do not serial print here, cause crash
 *
 */
void ISR_emergency_stop_down()
{
    if (spool.direction == down && spool.endstop_down == false)
    {
        spool.endstop_down = true;
        md.setM1Brake(400);
        if (VERBOSE_MOTOR){output.println("Endstop down touched - system stopped");}

        // TODO : system alert and user warning
    }
}