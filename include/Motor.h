#ifndef MOTOR_H
#define MOTOR_H

#include <Arduino.h>

// looking from motor to manifold:
// down : Clockwise
// up : Counterclockwise
enum motor_direction{down, up};

class Motor
{

private:
    byte control_pin;
    int speed; // Refers to power
    String ID = "no_ID";
    bool homed = false; // spool: the length is known (the origin switch was reached since the boot); the encoder counts from it
    bool drive(motor_direction dir, uint8_t fast_speed, float goal_cm, bool to_switch, const char **why);

public:
    motor_direction direction;
    
    bool endstop_down = false; // public for interrupt
    bool endstop_up = false;
    void begin();
    void begin(String _ID);
    void set_speed(uint8_t _speed, motor_direction _direction);
    void start();
    void start(uint8_t _speed, motor_direction _direction);
    void start(int _depth);
    void start_origin();
    // Spool (depth). Lengths are cm of tube unrolled from the origin switch. Both block until the move ends and return
    // false with a reason in *why (stall, wrong way, timeout, end-stop, x typed). A move never lasts more than its distance allows (SPOOL_MIN_CM_PER_S, SPOOL_MOVE_MARGIN_MS).
    bool move_to_length(float length_cm, const char **why); // rolls up to the origin first if the position is unknown
    bool go_home(const char **why);                         // rolls up to the origin switch
    bool is_homed() { return homed; }
    float length_cm();                                      // encoder length now (true only when is_homed())
    void stop();
    void stopIfFault();
};

void ISR_emergency_stop_down();
void ISR_emergency_stop_up();

#endif