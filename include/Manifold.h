#ifndef MANIFOLD_H
#define MANIFOLD_H

#include <Arduino.h>
#include "C_output.h"
#include "Motor.h"


//----- ENCODER -----//
/* SPI commands */
#define AMT22_NOP       0x00 /*get posiiton*/
#define AMT22_RESET     0x60
#define AMT22_ZERO      0x70
#define AMT22_TURNS     0xA0

/* Define special ascii characters */
#define NEWLINE         0x0A
#define TAB             0x09

/* We will use these define macros so we can write code once compatible with 12 or 14 bit encoders */
#define RES12           12

const float encoder_to_deg = 360.0 / 4096.0;

const float purge_angle =300.06;
// for the diff of both:
// * option 1
// const float purge_angle = 88.24; //is the angle at which the purge hole is aligned with the rotor hole
// * option 2
// const float purge_angle =156.97;

const float angle_between_slots = 22.5;

const int omitted_angle_nb = 12;

// ! make them const again after testing
extern float angle_offset_pos;// 3; //5.2 for 160 of speed
//const float angle_offset_neg = 4.9+6.3;
const float angle_offset_neg = -1.05;   // when turning CCW

enum slot_state
{
    unaivailable,
    available
};

class Manifold_slot{
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


class Manifold{
    private:
        Manifold_slot slots[NB_SLOT];
    public:
        void begin();
        void change_state(int i, slot_state state);
        slot_state get_state(int i);
        int get_id(int i);
        void reload();
};


void rotateMotor(int index); // angle: angle to reach
float scale_angle(float angle);
void directionDetermination(float goal_angle);
void readEncoder(bool init_setup);
uint8_t spiWriteRead(uint8_t sendByte, uint8_t encoder, uint8_t releaseLine);
uint16_t getPositionSPI(uint8_t encoder, uint8_t resolution);
uint16_t getRotationSPI(uint8_t encoder);
void setCSLine (uint8_t encoder, uint8_t csLine);

void calibrateEncoder(uint16_t speed = 40);
void setZeroSPI(uint8_t encoder);


#endif