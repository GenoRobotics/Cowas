#ifndef LINEAR_ACTUATOR_H
#define LINEAR_ACTUATOR_H

#include <Arduino.h>

enum actuator_direction
{
    forward,
    backward
};

// Controls a stepper (e.g. NEMA17 42SHD034-20B) through an A4988 driver in STEP/DIR mode.
class Linear_actuator
{
private:
    byte step_pin;
    byte dir_pin;
    byte enable_pin;
    uint16_t steps_per_rev;   // full steps per motor revolution (200 for a 1.8deg/step motor)
    uint8_t microsteps;       // microstepping set on the A4988 (MS1/MS2/MS3), 1 = full step
    float lead_mm_per_rev;    // linear travel per revolution of the lead screw, 0 if unknown/unused
    uint16_t step_delay_us;   // half-period of the step pulse, sets speed
    long position_steps;      // tracked position, steps from begin()/reset_position()
    actuator_direction current_direction; // direction last set via set_direction()

public:
    void begin(byte _step_pin, byte _dir_pin, byte _enable_pin,
               uint16_t _steps_per_rev = 200, uint8_t _microsteps = 1,
               float _lead_mm_per_rev = 0);
    void enable();
    void disable();
    void set_speed(uint16_t steps_per_second);
    void set_direction(actuator_direction dir); // sets DIR pin once; does not touch it again until called
    void step_pulses(long steps);               // pulses STEP using the direction set by set_direction()
    void move_steps(long steps, actuator_direction dir); // convenience: set_direction() + step_pulses()
    void move_mm(float mm, actuator_direction dir);
    long get_position_steps();
    void reset_position();
};

#endif
