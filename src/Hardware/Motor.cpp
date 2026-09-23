#include <Arduino.h>
#include "Motor.h"
#include "DualVNH5019MotorShield.h"
#include "Encoder.h"
#include "Settings.h"
#include "Button.h"
#include "Critical_error.h"
#include "C_output.h"

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
 * @brief Start the motor to go at a given absolute depth.
 *
 * @param _depth in centimeters. Negative to go out of water at origin.
 */
void Motor::start(int _depth)
{

    // if complete rewind, set a security distance to stop, then use start_origin function
    if (_depth <= 0)
        _depth = -HEIGHT_FROM_WATER + DISTANCE_FROM_STOP;
    if (_depth > TUBE_LENGTH)
        _depth = -HEIGHT_FROM_WATER + TUBE_LENGTH;

    depth_goal = _depth;                           // distance from water level
    int distance = depth_goal + HEIGHT_FROM_WATER; // absolute distance from sensor
    encoder.set_distance_to_reach(distance);
    // movement setup depending of absolute depth
    if (depth_goal > depth_current){            // ! very dangerous if saved state is wrong
        set_speed(SPEED_DOWN, down);}
    else if (depth_goal < depth_current){
        set_speed(SPEED_UP, up);}
    else{
        set_speed(0, down);}

    if (VERBOSE_MOTOR){output.println("Motor started to go at " + String(_depth));}

    // INFO | function can be accelerated and made more precises ATMEL hardware encoder core
    start();
    // loop take ~6 us.
    if (direction == down){
        while (encoder.get_distance() < distance){
            encoder.step_counter();
        }
    }
    else{
        while (encoder.get_distance() > distance){
            encoder.step_counter();
        }
    }
    stop();

    // update state
    depth_current = depth_goal;
    if (_depth <= 0)
        start_origin();
}

/**
 * @brief reset the motor to initial position at low speed using microswitch sensor
 *
 */
void Motor::start_origin()
{
    // check that sensor is working
    if (button_spool_up.getState() == 1)
    {
        set_speed(30, up);

        if (VERBOSE_MOTOR || VERBOSE_REWIND){output.println("Motor started to go at origin");}

        start();
        while (button_spool_up.getState() == 1)
            ;
        stop(); // should be useless as interrupt is taking care of stopping the motor
        encoder.reset();
        depth_current = -HEIGHT_FROM_WATER;
    }
    else
    {
        // output.println("Error with spool button | sensor not working");
        // critical_error(1);
        encoder.reset();
        depth_current = -HEIGHT_FROM_WATER;
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