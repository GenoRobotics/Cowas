/**
 * @file Sample.cpp
 * @author Timothée Hirt & Paco Mermoud
 * @brief Handle samples. There is a class "sample" to handle the samples as objects.
 *        Then there is global function to handle the sample array, containing all sample objects.
 * @version 0.1
 * @date 2022-01-29
 *
 * @copyright Copyright (c) 2022
 *
 */
#include <list>
#include "Sample.h"
#include "C_output.h"
#include "TimeLib.h"

extern C_output output;

std::list<Sample> samples; // list of samples
uint8_t filter_number = 2; // next sample on real system to be used. Filter number.


// ====== SAMPLE OBJECTS FUNCTIONS =========

/**
 * @brief Construct a new sample object
 *
 * @param _epoch The number of seconds since 01.01.1970
 * @param _depth The depth of the sample, in centimeters
 * @param _frequency The days after which the sample is repeated. 0 for unique sampling.
 */
Sample::Sample(time_t _epoch, int _depth, uint8_t _frequency)
{
    date.time.hour = hour(_epoch);
    date.time.minute = minute(_epoch);
    date.day = day(_epoch);
    date.month = month(_epoch);
    date.year = year(_epoch);
    date.epoch = _epoch;
    depth = _depth;
    frequency = _frequency;
    if (frequency == 0)
        type = unique;
    else
        type = repeated;
}

/**
 * @brief Construct a new sample object. A human constructor. Seconds aren't fun isn't it ?
 *
 * @param _hour
 * @param _minute
 * @param _day
 * @param _month
 * @param _year
 * @param _depth The depth of the sample, in centimeters
 * @param _frequency The days after which the sample is repeated.
 */
Sample::Sample(uint8_t _hour, uint8_t _minute, uint8_t _day, uint8_t _month, uint16_t _year, int _depth, uint8_t _frequency)
{
    Sample(timeToEpoch(_hour, _minute, _day, _month, _year), _depth, _frequency);
}

uint8_t Sample::get_day() { return date.day; }

uint8_t Sample::get_hour() { return date.time.hour; }

uint8_t Sample::get_minute() { return date.time.minute; }

uint8_t Sample::get_month() { return date.month; }

uint16_t Sample::get_year() { return date.year; }

time_t Sample::get_epoch() { return date.epoch; }

uint16_t Sample::get_depth() { return depth; }

uint8_t Sample::get_frequency() { return frequency; }


// ============= SAMPLE ARRAY GLOBAL FUNCTIONS

/**
 * @brief Add new sample to the list in time ordered manner
 *
 * @param _epoch The number of seconds since 01.01.1970
 * @param _depth The depth of the sample, in centimeters
 * @param _frequency The days after which the sample is repeated. 0 for unique sampling.
 * @return the index in the list of the sample (ordered in time)
 */
uint8_t add_sample(time_t _epoch, int _depth, uint8_t _frequency)
{

    Sample sample(_epoch, _depth, _frequency);
    uint8_t sample_nb = samples.size();
    bool inserted = false;
    uint8_t i = 0;

    if (samples.size() == 0)
    {
        samples.push_back(sample);
    }
    else
    {
        // initializing list iterator to beginning
        auto it = samples.begin();

        while (i < sample_nb && !inserted)
        {
            advance(it, i);
            // insert sample before next samples in time if nearer in time
            if (it->get_epoch() > sample.get_epoch())
            {
                samples.insert(it, sample);
                inserted = true;
            }
            else
                i++;
        }
        // if sample was not inserted -> furstest away in time, then insert at the end
        if (!inserted)
        {
            samples.push_back(sample);
        }
    }

    // return position of inserted sample
    return i;
}

/**
 * @brief A human way to add a sample.
 *
 * @param _hour
 * @param _minute
 * @param _day
 * @param _month
 * @param _year
 * @param _depth The depth of the sample, in centimeters
 * @param _frequency The days after which the sample is repeated. 0 for unique sampling.
 * @return the index in the list of the sample (ordered in time)
 */
uint8_t add_sample(uint8_t _hour, uint8_t _minute, uint8_t _day, uint8_t _month, uint16_t _year, int _depth, uint8_t _frequency)
{
    return add_sample(timeToEpoch(_hour, _minute, _day, _month, _year), _depth, _frequency);
}

/**
 * @brief Return the time (epoch) of the next sample.
 *
 * @return time_t The time in second since 1970. 0 if no sample.
 */
time_t get_next_sample_time()
{

    if (samples.size() == 0)
    {
        return 0;
    }
    else
    {
        auto it = samples.begin();
        return it->get_epoch();
    }
}

/**
 * @brief Return a sample object from the sample list.
 *
 * @param number The number of the sample in the list (starting from 0).
 * @return Sample
 */
Sample get_sample(uint8_t number)
{

    auto it = samples.begin();
    uint8_t list_size = samples.size();
    if (number >= list_size)
    {
        output.println("Unable to get requested sample, index out of range, taking last sample");
        number = list_size - 1;
    }
    advance(it, number);
    Sample sample(it->get_epoch(), it->get_depth(), it->get_frequency());

    return sample;
}

/**
 * @brief Display all the set samples.
 *
 */
void display_samples()
{
    auto it = samples.begin();

    output.println("Display samples command executed");

    if (samples.size() > 0)
    {
        for (uint8_t i = 0; i < samples.size(); i++)
        {
            String depth = String(it->get_depth());
            String frequency = String(it->get_frequency());
            time_t t = it->get_epoch();
            output.println("Sample" + String(i + 1) + " : " + format_date_friendly(t) + " at depth " + depth + "cm and day frequency " + frequency);
            advance(it, 1);
        }
    }
    else
    {
        output.println("No samples are currently set");
    }
}

/**
 * @brief Display the sample desired.
 *
 * @param number The list number of the sample (starting from 0)
 */
void display_sample(uint8_t number)
{

    if(VERBOSE_SAMPLE){output.println("Display sample command executed");}

    auto it = samples.begin();
    uint8_t list_size = samples.size();
    if (number >= list_size)
    {
        if(VERBOSE_SAMPLE){output.println("Unable to display requested sample, index out of range, taking last sample");}
        number = list_size - 1;
    }
    else if (list_size == 0)
    {
        if(VERBOSE_SAMPLE){output.println("No samples are currently set");}
    }
    advance(it, number);

    String depth = String(it->get_depth());
    String frequency = String(it->get_frequency());
    time_t t = it->get_epoch();
    if(VERBOSE_SAMPLE){output.println("Sample in filter " + String(filter_number) + " : " + format_date_friendly(t) + " at depth " + depth + "cm and day frequency " + frequency);}
    output.flush();
}

/**
 * @brief Function to call when a sampling was done.
 *        Delete the sample from the list and recreate one if a frequency is set.
 *
 */
void validate_sample()
{
    // log the sample
    display_sample(0);
    if(VERBOSE_SAMPLE){output.println("Ended at time " + format_date_logging(now()));}

    // set next sample on real hardware
    filter_number++;

    // if sample has a frequency, set it back in the list
    auto it = samples.begin();
    if (it->get_frequency() > 0)
    {
        Sample sample = get_sample(0);
        add_sample(sample.get_epoch() + sample.get_frequency() * SECS_PER_DAY, sample.get_depth(), sample.get_frequency());
    }

    // delete the sample done
    samples.pop_front();
}


