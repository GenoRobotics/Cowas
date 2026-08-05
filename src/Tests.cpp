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
#include "Micro_pump.h"
#include "Motor.h"
#include "Encoder.h"
#include "Led.h"
#include "C_output.h"
#include "Settings.h"
#include "Timer.h"
#include "Serial_device.h"
#include "Tests.h"
#include "Step_functions.h"
#include "Manifold.h"
#include "maintenance.h"
#include "Pressure_sensor.h"

extern int manifold_slot;
extern C_output output;
extern Serial_device serial;
extern Led status_led;
extern Led green_led;
extern Trustability_ABP_Gage pressure1;
extern Valve_2_2 valve_1;
extern Valve_3_2 valve_23;
extern Valve_2_2 valve_manifold;
extern Pump pump;

extern Micro_Pump micro_pump;

extern Motor spool;
extern Motor manifold_motor;
extern Encoder encoder;
extern Button button_start;    
extern Button button_container;
extern Button button_spool_up;     
extern Button button_spool_down;
extern Button button_left;      
extern Button button_right;
// extern Potentiometer potentiometer;
extern struct Timer timer_control_pressure1;
extern struct Timer timer_control_pressure2;

extern BigPressure pressure2;

void test_serial_device()
{
    if (Serial.available())
    {
        output.println(Serial.read());
        output.println("");
    }
}

void test_all_components(){
    // call all sub-functions

    // go_to_zero();

    String test_command;
    while (42){
        if (Serial.available()){
            // test_command = Serial.readString();
            test_command = Serial.readStringUntil('\n');
            //test_command = Serial.readStringUntil('b');
            Serial.println(test_command);

            if(test_command == "abort"){
                Serial.println("Abort: empty container, rewind and empty deplyoment"); 
                abort_sample();  
            }

            if (test_command == "temp"){    // quick debugging, change this function
                while (true){
                    if (Serial.available() > 0)
                    {
                        float d1;
                        d1 = Serial.parseFloat();
                        if (d1 == 0.0){
                            Serial.println("zerrooooooooooo");
                            continue;
                        }
                            
                        angle_offset_pos = d1;
                        Serial.println(d1);

                        // up is CCW, down is CW
                        manifold_motor.start(30, up);

                        while (!Serial.available()){
                            
                            delay(200);
                        }
                        manifold_motor.stop();

                        // go back to slot0 to check
                        rotateMotor(0);  
                        
                    }
                    delay(2000);
                }
                
            }
            
            if (test_command == "sample1m"){
                Serial.println("test Sampling at 1 meter, press START");
                button_start.waitPressedAndReleased();
                sample_process(1*80);
            }

            if (test_command == "demo"){
                Serial.println("Demo sampling");
                demo_sample_process();
            }

            if (test_command == "cal_DNA"){
                Serial.println("Cal DNA shield");
                calibrate_DNA_pump();
            }

        // all valves, both motors, pump, pressure sensors, push buttons(spool), 
        // leds, control buttons + potentiometer, container
            if (test_command == "valves"){
                // 1, 2, 3, 4 with a second delay each
                Serial.println("testing valves");
                test_valves();
            }
            if (test_command == "motor_spool"){
                Serial.println("testing motor");
                test_motor_spool();
            }
            if (test_command == "motor_manifold"){
                Serial.println("testing motor manifold");
                // test straight if manifold is working? or motor and encoder seperate and then together
                
            }
            if (test_command == "encoder_manifold"){
                Serial.println("testing encoder Manifold");
                test_encoder();
            }
            if (test_command == "enc_rot"){
                // Serial.println("");
                getRotationSPI(ENCODER_MANIFOLD);
            }
            if (test_command == "encoder_spool"){
                Serial.println("testing encoder SPOOL");
                test_encoder_spool();
            }
            if (test_command == "manifold"){
                Serial.println("testing manifold");
                test_manifold();    // all manifold, test first motor and encoder
            }
            if (test_command == "pressure_sensors"){
                Serial.println("testing pressure_sensors");
                test_pressure_sensor();
            }
            if (test_command == "pump"){
                Serial.println("testing pump");
                test_pump();
            }
            if (test_command == "micro_switch"){
                Serial.println("testing micro switch");
                test_micro_switch();
            }
            if (test_command == "buttons_command"){
                Serial.println("testing buttons");
                test_command_box();
                
            }
            if (test_command == "container"){
                Serial.println("testing container");
            }
            if (test_command == "spool"){
                Serial.println("testing spool");
                test_1_depth_20m();
            }
            if (test_command == "40m"){
                Serial.println("testing spool - 40m");
                test_1_depth_40m();
            }
            

            if (test_command == "stop"){
                Serial.println("stop testing mode");
                break;
            }

            if (test_command == "cal"){
                Serial.println("Calibrating encoder Manifold");
                calibrateEncoder(20);       // ! maybe need change
            }

            if (test_command == "zero"){
                // Serial.println("Calibrating encoder Manifold");
                go_to_zero();       // ! maybe need change
            }

            if (test_command == "slot0"){
                // Serial.println("Calibrating encoder Manifold");
                rotateMotor(0);       // ! maybe need change
            }

            if (test_command == "micro_pump"){
                Serial.println("Testing micro pumps");
                micro_pump.start();
                delay(10000);
                micro_pump.stop();
            }

        }
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

    while (!button_start.isPressed())
    {
        if (button_left.isPressed())
        {
            button_left.waitPressedAndReleased();
            pump.set_power(100);
            pump.start();
        }
        if (button_right.isPressed())
        {
            button_right.waitPressedAndReleased();
            pump.set_power(100);
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
    // int pot_last_value = potentiometer.get_value(0, 100);
    // int pot_value = 0;
    // int speedy = 60;
    while (!button_start.isPressed())
    {
        // pot_value = potentiometer.get_value(0, 100);
        // if (pot_value <= pot_last_value - 4 || pot_value >= pot_last_value + 4)
        // {
        //     // speedy = pot_value;
        //     // pot_last_value = pot_value;
        //     output.println("speed " + String(speedy));
        // }
        if (button_left.isPressed())
        {
            button_left.waitPressedAndReleased();
            pump.set_power(100);
            pump.start();
        }
        if (button_right.isPressed())
        {
            button_right.waitPressedAndReleased();
            pump.set_power(100);
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
    float pressureBig;

    while (!Serial.available()){
        delay(500); // don't read pressure to fast
        pressure = pressure1.getPressure();
        pressureBig = pressure2.readPressure();


        Serial.print("Pressure Trustability value : ");
        Serial.println(pressure);
        Serial.print("Pressure New sensor value : ");
        Serial.println(pressureBig);
        // Serial.print("   Analog value: ");
        // Serial.println(analogRead(pressure_2_pin));

        delay(500);
    }
}

void test_manifold(){
    uint16_t pos;
    float angle_deg;

    for (int j = 1  ; j < 15; j++)
        {
        rotateMotor(0);
        delay(1000);
        rotateMotor(j);

        pos = getPositionSPI(ENCODER_MANIFOLD, RES12);
        angle_deg = pos * encoder_to_deg;
        Serial.print("Manifold encoder value : ");
        Serial.print(pos);
        Serial.print(",    Angle in degrees : ");
        Serial.print(angle_deg);
        Serial.print(",    Slot number : ");
        Serial.println(j);

        delay(3000);
        }
}

void test_pump(){
    // open manifold valve to avoid building up pressure
    valve_manifold.set_open_way();

    pump.set_power(100);
    pump.start(2000);       // running for 2s
    pump.set_power(50);
    pump.start(2000); 

    valve_manifold.set_close_way();
}

void test_motor_spool(){
    spool.set_speed(20, down);
    spool.start();
    delay(1000);
    spool.stop();
    delay(500);
    spool.set_speed(20, up);
    spool.start();
    delay(1000);
    spool.stop();
}

void test_valves(){
    // extern valve_1;valve_23; valve_manifold;
    for (uint8_t i = 0; i < 2; i++){
        Serial.println("loop " + String(i));
        valve_1.switch_way();
        delay(5000);
        valve_23.switch_way();
        delay(5000);
        valve_manifold.switch_way();
        delay(5000);
    }
}


void test_command_box(){
    bool start_last = button_start.isPressed();
    bool left_last = button_left.isPressed();
    bool right_last = button_right.isPressed();
    bool start;
    bool left;
    bool right;

    green_led.off();
    status_led.on();
    uint32_t last_led_switch = millis();
    // uint32_t last_pot_print = millis();

    while (!Serial.available()){
        if (last_led_switch + 1000 < millis()){
            green_led.switch_state();
            status_led.switch_state();
            last_led_switch = millis();
        }

        start = button_start.isPressed();
        left = button_left.isPressed();
        right = button_right.isPressed();

        if (start_last != start){
            Serial.print("Button start ");
            Serial.println((start) ? "Pressed" : "Released");
        }

        if (left_last != left){
            Serial.print("Button left ");
            Serial.println((left) ? "Pressed" : "Released");
        }

        if (right_last != right){
            Serial.print("Button right ");
            Serial.println((right) ? "Pressed" : "Released");
        }

        start_last = start;
        left_last = left;
        right_last = right;

    }
}

void test_micro_switch(){
    bool up_last = button_spool_up.getState();
    bool down_last = button_spool_down.getState();
    bool container_last = button_container.getState();
    bool up;
    bool down;
    bool container;

    while (!Serial.available()){

        // the switches are normally pressed
        up = button_spool_up.getState();
        down = button_spool_down.getState();
        container = button_container.getState();

        if (up_last != up){
            Serial.print("Micro-switch spool up :");
            Serial.println((!up) ? "Pressed" : "Released");
        }

        if (down_last != down){
            Serial.print("Micro-switch spool down ");
            Serial.println((!down) ? "Pressed" : "Released");
        }

        if (container_last != container){
            Serial.print("Micro-switch container ");
            Serial.println((!container) ? "Pressed" : "Released");
        }

        up_last = up;
        down_last = down;
        container_last = container;
    }
}

void test_encoder(){
    uint16_t pos;
    float angle_deg;

    manifold_motor.start(30, down);

    while (!Serial.available()){
        pos = getPositionSPI(ENCODER_MANIFOLD, RES12);
        angle_deg = pos * encoder_to_deg;
        Serial.print("Manifold encoder value : ");
        Serial.print(pos);
        Serial.print(",    Angle in degrees : ");
        Serial.println(angle_deg);
        delay(500);
    }
    manifold_motor.stop();
}

void test_encoder_spool(){
    uint32_t last_print = millis();
    while (!Serial.available())
    {
        encoder.step_counter();
        if (last_print + 3000 < millis()){
            Serial.print("Pulses encoder : ");
            Serial.println(encoder.get_pulses_A());
            Serial.print("Depth : ");
            Serial.println(encoder.get_distance());

            last_print = millis();
        }
    }
    
}

void go_to_zero(){
    Serial.println("Going to zero manifold");
    // SETUP//
    bool end_rotation = false;
    float angle_to_reach = 0;

    // readEncoder(true);
    // readEncoder(false);
    float current_angle;

    manifold_motor.start(40, down);
    //ROTATE//
    while (end_rotation == false)
    {
        current_angle = getPositionSPI(ENCODER_MANIFOLD, RES12) * encoder_to_deg;

        if(VERBOSE_MANIFOLD){
            output.print("   Angle to reach: ");
            output.print(angle_to_reach);
            output.println(2);
        }

        if ((current_angle >= angle_to_reach - 5) && (current_angle <= angle_to_reach + 5))
        {
            manifold_motor.stop();
            end_rotation = true;
        }

        delay(1);//smaller delay -> better precision
    }
}

// ! to implement, only needed if encoder manifold still changes absolut 0
void reset_encoder(){
    Serial.println("Resetting ");
}