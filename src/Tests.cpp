/**
 * @file Tests.cpp
 * @author Timothée Hirt & Paco Mermoud
 * @brief Try things, be free. No guarantie that those functions work.
 * @version 0.1
 * @date 2022-01-29
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include <Arduino.h>
#include "Button.h"
#include "Trustability_ABP_Gage.h"
#include "Valve_3_2.h"
#include "Valve_2_2.h"
#include "Pump.h"
#include "Motor.h"
#include "Encoder.h"
#include "Potentiometer.h"
#include "Led.h"
#include "C_output.h"
#include "Settings.h"
#include "Timer.h"
#include "Serial_device.h"
#include "Tests.h"
#include "Step_functions.h"
#include "Manifold.h"

extern int manifold_slot;
extern C_output output;
extern Serial_device serial;
extern Led status_led;
extern Led green_led;
extern Trustability_ABP_Gage pressure1;
extern Trustability_ABP_Gage pressure2;
extern Valve_2_2 valve_1;
extern Valve_3_2 valve_23;
extern Valve_2_2 valve_manifold;
extern Pump pump;
extern Motor spool;
extern Encoder encoder;
extern Button button_start;    
extern Button button_container;
extern Button button_spool_up;     
extern Button button_spool_down;
extern Button button_left;      
extern Button button_right;
extern Potentiometer potentiometer;
extern struct Timer timer_control_pressure1;
extern struct Timer timer_control_pressure2;

void tests()
{

    if (serial.available())
    {
        output.println(serial.receive());
    }
    // output.println(pressure1.getPressure());
    // output.println(pressure1.getTemperature());
    // delay(500);
}


void test_serial_device()
{
    if (Serial1.available())
    {
        output.println(Serial1.read());
        output.println("");
    }
}

void test_pressure()
{

    int pot_last_value = potentiometer.get_value(0, 100);
    int pot_value = 0;
    int speedy = 60;
    while (!button_start.isPressed())
    {
        pot_value = potentiometer.get_value(0, 100);
        if (pot_value <= pot_last_value - 4 || pot_value >= pot_last_value + 4)
        {
            speedy = pot_value;
            pot_last_value = pot_value;
            output.println("speed " + String(speedy));
        }
        if (button_left.isPressed())
        {
            button_left.waitPressedAndReleased();
            pump.set_power(speedy);
            pump.start();
        }
        if (button_right.isPressed())
        {
            pump.stop();
            button_right.waitPressedAndReleased();
        }

        output.println("P1 " + String(pressure1.getPressure()) + "  |   P2 " + String(pressure2.getPressure()));
        delay(200);
    }
}

void test_1_depth_20m()
{
    output.println("TEST 1 - ALTIMETRE 20");
    uint32_t lastTime = 0;



    spool.start_origin();
    status_led.on();
    output.println("=========== go 30cm");
    button_start.waitPressedAndReleased();
    status_led.off();
    lastTime = millis();
    spool.start(30);
    output.println(millis()-lastTime);

    status_led.on();
    output.println("=========== go 1m");
    button_start.waitPressedAndReleased();
    status_led.off();
    lastTime = millis();
    spool.start(100);
    output.println(millis()-lastTime);

    status_led.on();
    output.println("=========== go 2m");
    button_start.waitPressedAndReleased();
    status_led.off();
    lastTime = millis();
    spool.start(200);
    output.println(millis()-lastTime);

    int i = 1;
    while (i < 5)
    {
        status_led.on();
        output.println("=========== go " + String(i*5) + "m");
        button_start.waitPressedAndReleased();
        status_led.off();
        lastTime = millis();
        spool.start(i*5*100);
        output.println(millis()-lastTime);
        i++;
    }

    i = 4;
    while (i > 0)
    {
        status_led.on();
        output.println("=========== go " + String(i*5) + "m");
        button_start.waitPressedAndReleased();
        status_led.off();
        lastTime = millis();
        spool.start(i*5*100);
        output.println(millis()-lastTime);
        i--;
    }

    status_led.on();
    output.println("=========== go origin");
    button_start.waitPressedAndReleased();
    status_led.off();
    lastTime = millis();
    spool.start(-1);
    output.println(millis()-lastTime);

    green_led.on();
    output.println("END OF TEST 1 20 meters");
    button_start.waitPressedAndReleased();
    green_led.off();
}

void test_1_depth_40m()
{
    output.println("TEST 1 - ALTIMETRE 40 indirect");
    uint32_t lastTime = 0;

    spool.start_origin();
    status_led.on();
    output.println("=========== go 20m");
    button_start.waitPressedAndReleased();
    status_led.off();
    lastTime = millis();
    spool.start(2000);
    output.println(millis()-lastTime);

    int i = 5;
    while (i < 10)
    {
        status_led.on();
        output.println("=========== go " + String(i*5) + "m");
        button_start.waitPressedAndReleased();
        status_led.off();
        lastTime = millis();
        spool.start(i*5*100);
        output.println(millis()-lastTime);
        i++;
    }

    i = 9;
    while (i > 3)
    {
        status_led.on();
        output.println("=========== go " + String(i*5) + "m");
        button_start.waitPressedAndReleased();
        status_led.off();
        lastTime = millis();
        spool.start(i*5*100);
        output.println(millis()-lastTime);
        i--;
    }

    status_led.on();
    output.println("=========== go origin");
    button_start.waitPressedAndReleased();
    status_led.off();
    lastTime = millis();
    spool.start(-1);
    output.println(millis()-lastTime);

    green_led.on();
    output.println("END OF TEST 1 40 meters");
    button_start.waitPressedAndReleased();
    green_led.off();
}

void test_1_depth_40m_direct()
{
    output.println("TEST 1 - ALTIMETRE 19m direct");
    uint32_t lastTime = 0;

    spool.start_origin();
    status_led.on();
    output.println("=========== go 19m");
    button_start.waitPressedAndReleased();
    status_led.off();
    lastTime = millis();
    spool.start(19000);
    output.println(millis()-lastTime);

    status_led.on();
    output.println("=========== go origin");
    button_start.waitPressedAndReleased();
    status_led.off();
    lastTime = millis();
    spool.start(-1);
    output.println(millis()-lastTime);

    green_led.on();
    output.println("END OF TEST 1 19 meters direct");
    button_start.waitPressedAndReleased();
    green_led.off();
}

void test_2_remplissage_container_1m(){

    output.println("TEST 2 - container 1m");

    spool.start(100);
    valve_23.set_I_way();
    delay(200);
    pump.set_power(255);
    timerStart(timer_control_pressure2);
    pump.start();
    while(button_container.getState() == 1);
    pump.stop();
    timerStop(timer_control_pressure2);
    valve_23.set_L_way();

    step_purge();
    step_rewind();

}

void test_2_remplissage_container_40m(){

    output.println("TEST 2 - container 40m");

    spool.start(40000);
    valve_23.set_I_way();
    delay(200);
    pump.set_power(100);
    timerStart(timer_control_pressure2);
    pump.start();
    while(button_container.getState() == 1);
    pump.stop();
    timerStop(timer_control_pressure2);
    valve_23.set_L_way();

    step_purge();
    step_rewind();
}

void test_3_sterivex_1(){

    output.println("TEST 3 - sterivex 1");

    spool.start(1000);
    for(int i = 0; i<3; i++){
        output.println("Fill container number " + String(i+1));
        step_fill_container();
        output.println("Purge number " + String(i+1));
        step_purge();
    }

    output.println("Fill container number 4");
    step_fill_container();

    // sampling sterivex 1
    uint32_t lastTime = millis();
    step_sampling(1);
    output.println("Temps " + String(millis()-lastTime));

    output.println("Control manuel pompe");
    status_led.on();
    button_start.waitPressedAndReleased();
    status_led.off();

    // mnaual contorl
    int pot_last_value = potentiometer.get_value(0, 100);
    int pot_value = 0;
    int speedy = 60;
    while (!button_start.isPressed())
    {
        pot_value = potentiometer.get_value(0, 100);
        if (pot_value <= pot_last_value - 4 || pot_value >= pot_last_value + 4)
        {
            speedy = pot_value;
            pot_last_value = pot_value;
            output.println("speed " + String(speedy));
        }
        if (button_left.isPressed())
        {
            button_left.waitPressedAndReleased();
            pump.set_power(speedy);
            pump.start();
        }
        if (button_right.isPressed())
        {
            button_right.waitPressedAndReleased();
            pump.set_power(speedy);
            pump.stop();           
        }
        delay(10);
    }
    button_start.waitPressedAndReleased();
    output.println("purge");
    green_led.on();
    button_start.waitPressedAndReleased();
    green_led.off();
    
    step_purge();
    output.println("rewind");
    button_start.waitPressedAndReleased();
    step_rewind();

}

void test_3_sterivex_2(){


    output.println("TEST 3 - sterivex 2");

    spool.start(1000);
    output.println("Fill container number 5");
    step_fill_container();

    // sampling sterivex 2
    uint32_t lastTime = millis();
    step_sampling(2);
    output.println("Temps " + String(millis()-lastTime));

    output.println("Control manuel pompe");
    status_led.on();
    button_start.waitPressedAndReleased();
    status_led.off();

    // mnaual contorl
    int pot_last_value = potentiometer.get_value(0, 100);
    int pot_value = 0;
    int speedy = 60;
    while (!button_start.isPressed())
    {
        pot_value = potentiometer.get_value(0, 100);
        if (pot_value <= pot_last_value - 4 || pot_value >= pot_last_value + 4)
        {
            speedy = pot_value;
            pot_last_value = pot_value;
            output.println("speed " + String(speedy));
        }
        if (button_left.isPressed())
        {
            button_left.waitPressedAndReleased();
            pump.set_power(speedy);
            pump.start();
        }
        if (button_right.isPressed())
        {
            button_right.waitPressedAndReleased();
            pump.set_power(speedy);
            pump.stop();           
        }
        delay(10);
    }
    button_start.waitPressedAndReleased();
    output.println("purge");
    green_led.on();
    button_start.waitPressedAndReleased();
    green_led.off();
    
    step_purge();
    output.println("rewind");
    button_start.waitPressedAndReleased();
    step_rewind();

}


void test_pressure_sensor(){
    float pressure;
    bool run = true;
    int compteur=0;
     do
    {
        delay(10); // don't read pressure to fast
        pressure = pressure1.getPressure();
        
        if (compteur==10)
        {
            output.println("Pressure = " + String(pressure));
            compteur=0;
        }

    compteur++;
    } while (run); // conditions outside while loop to allow printing which condition is responsible for stop

}

void test_manifold(){
    for (int j = 1  ; j < 15; j++)
        {
        rotateMotor(0);
        delay(5000);
        rotateMotor(j);
        delay(5000);

        }
        // rotateMotor(12);
        // delay(5000);
        // step_purge();
        // delay(5000);
        // rotateMotor(4);
        // delay(5000);
        // step_purge();
        // delay(5000);
        // rotateMotor(0);
        // delay(5000);
        // step_purge();
        delay(600000);
}

void add_button_manifold_demo(){
    if (button_start.isPressed())
    {
        rotateMotor(manifold_slot);
        valve_manifold.set_open_way();
        valve_23.set_L_way();
        delay(100);
        pump.set_power(POWER_FLUSH);
        pump.start(30000);
        delay(100);
        valve_manifold.set_close_way();
        manifold_slot++;
    }
}