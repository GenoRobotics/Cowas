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
// The reference is raw 0, which is human slot 7 (the table walks from the purge at raw 8).
// So `cal` is done with the rotor on slot 7 (2026-10-22). Not the purge hole.
const float purge_angle = 344.27; // D40 (F191): 276.77 + 67.5 (3 slots). The rotor was aligned to the table, but the reference sat on slot 10: `slot7` landed on physical slot 10. Eye-checked by the user 2026-10-23. Replaces 276.77 (D39, did not fix it); was 276.42 (encoder 3145.00, D37); was 275.45 (encoder 3134.00, D35); was 279.58 (encoder 3181.00, D34, loose encoder); was 345.06 (cal 2026-10-22, encoder 3926.00, D33, loose encoder); was 270.44 (cal 2026-10-20, encoder 3077.00, F165); was 341.37 (cal 2026-10-12, encoder 3884.00)
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

// F129: the wrap point of scale_angle(). rotateMotor() takes the direction from the sign of
// (scaled goal - scaled current angle), so a move never crosses the wrap point: it goes the long way
// round instead. It sits on the no-hole position (between slots 3 and 4), where no slot is.
// scale_angle() wraps at +/-180 after adding half a pitch, hence the two terms.
const float scale_reference_angle = purge_angle - omitted_angle_nb * angle_between_slots + 0.5f * angle_between_slots - 180.0f;

// Raw steps from purge_angle (the calibrated reference, physically "top") to the
// actual purge port (physically "bottom", 180 degrees away = half of 16 steps).
// sterivex_angle[] is built starting from this offset, so index 0 = purge and
// 1..14 = samples (see Manifold::begin()).
const int PURGE_RAW_OFFSET = 8;

const uint16_t MANIFOLD_CAL_SPEED = 20; // slow but confirmed to actually move the motor (matches the previously-working value; below ~20 the motor doesn't have enough torque to turn at all)

// ! make them const again after testing
extern float angle_offset_pos; // 3; //5.2 for 160 of speed
// const float angle_offset_neg = 4.9+6.3;
const float angle_offset_neg = 2.5; // when turning CCW. 2026-10-22 (user): rotate_all dry, CCW moves mean -1.28 deg overshoot, so 1.2 + 1.28 (F128 history: was -1.05 at speed 40, 2026-10-12). Check with rotate_all dry order=fill auto

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

// FW2 encoder helpers (single reads, 3 attempts; no motion)
bool read_manifold_angle(float *angle_deg);                    // raw encoder angle 0..360, false on a bad read
int manifold_nearest_slot(float *error_deg, bool *encoder_ok); // nearest slot + signed error, -1 on encoder failure
bool manifold_slot_error(int slot, float *error_deg);          // signed error to `slot`, false on encoder failure
void directionDetermination(float goal_angle);
bool readEncoder(bool init_setup); // returns false if the encoder read failed (bad checksum after retries) - current_angle is left unchanged in that case
uint8_t spiWriteRead(uint8_t sendByte, uint8_t encoder, uint8_t releaseLine);
uint16_t getPositionSPI(uint8_t encoder, uint8_t resolution);
uint16_t getRotationSPI(uint8_t encoder);
void setCSLine(uint8_t encoder, uint8_t csLine);

void calibrateEncoder(uint16_t speed = MANIFOLD_CAL_SPEED);
void setZeroSPI(uint8_t encoder);

#endif