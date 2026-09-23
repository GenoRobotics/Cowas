#ifndef FLOW_SENSOR_H
#define FLOW_SENSOR_H

#include <Arduino.h>

// void flow_setup();
// void print_flow();
// void read_print_pres();
// void flow_loop();

const uint8_t flow_small = 12;
const uint8_t flow_big = 13;

const float calibrationFactor_small = 77.0;
const float calibrationFactor_big = 70.0;

typedef void (* interruptRoutine)(void);

extern volatile uint32_t pulseCount_small;
extern volatile uint32_t pulseCount_big;

// interupt routines
void pulseCounter_small();
void pulseCounter_big();

class Flow_sensor
{

private:
    byte _pin;
    uint16_t _pulsePerL;
    float _flowRate;             // not needed now, but will to detect if sample is saturated
    //unsigned int _flowMilliLitres;
    float _totalMilliLitres;
    volatile uint32_t* _pulseCount;
    interruptRoutine _pulseCounter;
    uint32_t _lastUpdate;

public:
    void begin(byte pin, uint8_t cal_factor, volatile uint32_t* pulseCount, interruptRoutine pulseCounter);
    void update();      // reads count number and updates flow rate and milliL, need to call get functions after
    void activate();    // enable interupt routine
    void deactivate();  // disable interupt routine
    float get_flowRate();       // not needed now
    unsigned int get_totalFlowMilliL();
    void reset_values();
    void print();
};

#endif