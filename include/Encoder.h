#ifndef ENCODER_H
#define ENCODER_H

#include <Arduino.h>

enum encoder_direction{e_down, e_up};

class Encoder 
{

private:
    byte pin_signal_A;      // do X pulses per round
    byte pin_signal_B;      // do X puslses per round, phase shifted by 90° electric
    byte pin_signal_Z;      // do a pulse each round
    int pulse_per_rev;      // number of pulse in one turn for signal A and B. Datasheet.
    float rad_per_pulse;    // PI divided by pulse_per_rev
    float diameter;         // diameter of spool in centimeter
    int32_t pulses_A;       // total pulse counter
    int32_t pulses_Z;       // total round counter
    int32_t pulses_B;
    int32_t goal_pulses_A;  // pulses to reach
    encoder_direction direction;
    int depth;              // depth to reach, in centimeter
    String ID = "no_ID";

public:
    void begin(byte _pin_signal_A, byte _pin_signal_B, byte _pin_signal_Z, const int _pulse_per_rev, const float _diameter);
    void begin(byte _pin_signal_A, byte _pin_signal_B, byte _pin_signal_Z, const int _pulse_per_rev, const float _diameter, String _ID);
    int get_distance();
    int32_t get_goal_pulses();
    int32_t get_pulses_A();
    int32_t get_pulses_B();
    int32_t get_pulses_Z();
    void set_distance_to_reach(int distance);
    void set_pulses_Z(encoder_direction _direction);
    encoder_direction get_direction();
    int32_t step_counter();
    void reset();
    void print_states();
};



#endif