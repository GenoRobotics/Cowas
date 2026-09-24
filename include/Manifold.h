#ifndef MANIFOLD_H
#define MANIFOLD_H

#include <Arduino.h>
#include "C_output.h"
#include "Motor.h"

//----- ENCODER -----//
/* SPI commands */
#define AMT22_NOP 0x00 /*get posiiton*/
#define AMT22_RESET 0x60
#define AMT22_ZERO 0x70
#define AMT22_TURNS 0xA0

/* Define special ascii characters */
#define NEWLINE 0x0A
#define TAB 0x09

/* We will use these define macros so we can write code once compatible with 12 or 14 bit encoders */
#define RES12 12

const float encoder_to_deg = 360.0 / 4096.0;

// purge_angle is the CALIBRATED REFERENCE point (calibrate here with `cal`), not
// necessarily the purge hole itself - the purge hole sits PURGE_RAW_OFFSET raw
// steps away from this reference (see below). Historically the two were the same
// physical spot; they no longer have to be.
const float purge_angle = 342.07;
// for the diff of both:
// * option 1
// const float purge_angle = 88.24; //is the angle at which the purge hole is aligned with the rotor hole
// * option 2
// const float purge_angle =156.97;

const float angle_between_slots = 22.5;

// Raw positions around the rotor (360 / angle_between_slots): 15 holes + 1 no-hole
// position. The slot math lives in src/Core/manifold_geometry.cpp (unit-tested with
// `pio test -e native`); Manifold.cpp checks NB_SLOT against this at compile time.
const int MANIFOLD_RAW_POSITIONS = 16;

const int omitted_angle_nb = 12; // raw steps from purge_angle (the calibration reference) to the no-hole position

// Raw steps from purge_angle (the calibrated reference, physically "top") to the
// actual purge port (physically "bottom", 180 degrees away = half of 16 steps).
// sterivex_angle[] is built starting from this offset, so index 0 = purge and
// 1..14 = samples (see Manifold::begin()).
const int PURGE_RAW_OFFSET = 8;

const uint16_t MANIFOLD_CAL_SPEED = 20; // slow but confirmed to actually move the motor (matches the previously-working value; below ~20 the motor doesn't have enough torque to turn at all)

// ! make them const again after testing
extern float angle_offset_pos; // 3; //5.2 for 160 of speed
// const float angle_offset_neg = 4.9+6.3;
const float angle_offset_neg = -1.05; // when turning CCW

enum slot_state
{
    unaivailable,
    available
};

class Manifold_slot
{
private:
    int pin_control;
    slot_state state; // 1 available, 0 unaivailable
    int ID;

public:
    void begin(byte _pin_control, int _id);
    void change_state(slot_state state);
    slot_state get_state();
    int get_id();
};

class Manifold
{
private:
    Manifold_slot slots[NB_SLOT];

public:
    void begin();
    void change_state(int i, slot_state state);
    slot_state get_state(int i);
    int get_id(int i);
    void reload();
};

bool rotateMotor(int index); // angle: angle to reach - returns false if it had to abort (encoder failure or timeout), true on a normal completion

// Clock-face labeling for slots, purely a human-friendly alias for rotateMotor()'s
// numeric slot index - top of the manifold = "00" (12 o'clock), minutes increase
// around the face the way a clock's minute hand does. See MANIFOLD_RAW_INDEX_INCREASES_CW
// (Settings.h) for the physical-direction caveat.
int clock_minutes_for_slot(int human_slot);    // human_slot 0-14 (0=purge) -> minutes 0-59, or -1 if invalid
int slot_for_clock_minutes(int minutes);       // minutes -> human_slot, or -1 if no slot has that label

float scale_angle(float angle);
void directionDetermination(float goal_angle);
bool readEncoder(bool init_setup); // returns false if the encoder read failed (bad checksum after retries) - current_angle is left unchanged in that case
uint8_t spiWriteRead(uint8_t sendByte, uint8_t encoder, uint8_t releaseLine);
uint16_t getPositionSPI(uint8_t encoder, uint8_t resolution);
uint16_t getRotationSPI(uint8_t encoder);
void setCSLine(uint8_t encoder, uint8_t csLine);

void calibrateEncoder(uint16_t speed = MANIFOLD_CAL_SPEED);
void setZeroSPI(uint8_t encoder);

#endif