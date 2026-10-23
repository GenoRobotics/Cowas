#ifndef SAMPLE_H
#define SAMPLE_H


#include <Arduino.h>
#include "Settings.h"

enum Sampling_type
{
    unique,
    repeated
};


// =============== Sample object ==================

class Sample
{

private:
    static uint8_t sample_number;   // the next sample to be used
    struct Date date;               // store hour of sampling
    int depth;                 // depth of the sample in cm
    uint8_t frequency;              // frequency of sampling if sampling is regular
    Sampling_type type;             // if sample is regular or unique
   
public:
    Sample(time_t _epoch, int _depth, uint8_t _frequency);
    Sample(uint8_t _hour, uint8_t _minute,  uint8_t _day, uint8_t _month, uint16_t _year, int _depth, uint8_t _frequency);
    uint8_t get_day();
    uint8_t get_hour();
    uint8_t get_minute();
    uint8_t get_month();
    uint16_t get_year();
    time_t get_epoch();
    uint16_t get_depth();
    uint8_t get_frequency();
};

// =============== SampleS manipulation ==================

uint8_t add_sample(time_t _epoch, int _depth, uint8_t _frequency);
uint8_t add_sample(uint8_t _hour, uint8_t _minute,  uint8_t _day, uint8_t _month, uint16_t _year, int _depth, uint8_t _frequency);
time_t timeToEpoch(uint8_t _hour, uint8_t _minute, uint8_t _day, uint8_t _month, uint16_t _year);
time_t get_next_sample_time();
Sample get_sample(uint8_t number);
void display_samples();
void display_sample(uint8_t number);
void validate_sample();

// ============ filter on real system management =========

void reload_filters(uint8_t number_of_filter);
bool is_filter_available();
uint8_t get_next_filter_place();

#endif