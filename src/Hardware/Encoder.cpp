#include <Arduino.h>
#include "Encoder.h"
#include <math.h>
#include "C_output.h"

extern C_output output;
extern Encoder encoder;

void Encoder::begin(byte _pin_signal_A, byte _pin_signal_B, byte _pin_signal_Z, const int _pulse_per_rev, const float _diameter)
{
    pin_signal_A = _pin_signal_A;
    pin_signal_B = _pin_signal_B;
    pin_signal_Z = _pin_signal_Z;
    pulse_per_rev = _pulse_per_rev;
    diameter = _diameter;
    pinMode(pin_signal_A, INPUT);
    pinMode(pin_signal_B, INPUT);
    pinMode(pin_signal_Z, INPUT);

    rad_per_pulse = PI / pulse_per_rev;
    
    output.println("Encoder sensor " + ID + " initiated");
}

void Encoder::begin(byte _pin_signal_A, byte _pin_signal_B, byte _pin_signal_Z, const int _pulse_per_rev, const float _diameter, String _ID)
{
    ID = _ID;
    begin(_pin_signal_A, _pin_signal_B, _pin_signal_Z, _pulse_per_rev, _diameter);
}

/**
 * @brief set distance in pulses to reach
 *
 * @param _distance in centimeters
 */
void Encoder::set_distance_to_reach(int _distance)
{
    goal_pulses_A = _distance / (rad_per_pulse * diameter);
}

void Encoder::set_pulses_Z(encoder_direction _direction)
{
    if (_direction == e_up)
        pulses_Z--;
    else
        pulses_Z++;
}

int32_t Encoder::get_pulses_A() { return pulses_A; }

int32_t Encoder::get_pulses_Z() { return pulses_Z; }

int32_t Encoder::get_pulses_B() { return pulses_B; }

encoder_direction Encoder::get_direction() { return direction; }

int32_t Encoder::get_goal_pulses() { return goal_pulses_A; }

int Encoder::get_distance() { 
    //Serial.print("\nEncoder distance: "+String(rad_per_pulse * diameter * pulses_A)+ " ,  cast in int ="+ String((int)(rad_per_pulse * diameter * pulses_A)));
    return (int)(rad_per_pulse * diameter * pulses_A); }

/**
 * @brief Step counter of rotary encoder
 *        As we work at very low speed, this is enough.
 *        Function can be accelerated with direct microprocessor control as Arduino Due
 *        integrate an inbuilt quadrature encoder
 *        See datasheet page 871 or "36.6.14 Quadrature Decoder"
 *
 * @return int32_t the value of the step. equivalent to get_pulses_A
 */
int32_t Encoder::step_counter()
{
    static int A_state;
    static int A_last_state = digitalRead(pin_signal_A);

    A_state = digitalRead(pin_signal_A);
    if (A_state != A_last_state)
    {
        if (digitalRead(pin_signal_B) != A_state)
        {
            pulses_A++;
            direction = e_down;
        }
        else
        {
            pulses_A--;
            direction = e_up;
        }
    }
    A_last_state = A_state;
    return pulses_A;
}

void Encoder::reset()
{
    pulses_A = 0;
    pulses_B = 0;
    pulses_Z = 0;
}

/**
 * @brief For debugging purposes.
 *
 */
void Encoder::print_states()
{
    output.println("Pulses A : " + String(encoder.get_pulses_A()));
    output.println("Pulses B : " + String(encoder.get_pulses_B()));
    output.println("Pulses Z : " + String(encoder.get_pulses_Z()));
    output.println("Distance : " + String(encoder.get_distance()));
    output.println("Direction : " + String(encoder.get_direction() == e_up ? "up" : "down"));
}