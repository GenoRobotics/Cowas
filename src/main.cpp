/**
 * @file MAIN.CPP
 * @author Timothée Hirt & Paco Mermoud
 * @brief A GenoRobotics project - CoWaS (Continous Water Sampling). 
 *        A project for ARDUINO DUE
 * @version 2.0
 * @date 2022-01-28
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
#include <SPI.h>
#include "C_output.h"
#include "Settings.h"
#include "Critical_error.h"
#include "Timer.h"
#include "Serial_device.h"
#include "Tests.h"
#include "Step_functions.h"
#include "GPIO.h"
#include "TimeLib.h"
#include "Manifold.h"
#include "maintenance.h"
#include "experiences.h"

#include "Flow_sensor.h"
#include "Pressure_sensor.h"

// ============ MAIN FUNCTION DECLARATION =======
void main_program();
void button_control();
void SIC_control();


// ============ EXECUTION MODE ===================
// comment this line if no system check at startup
#define  SYSTEM_CHECKUP
// ============ PIN DEFINITIONS ==================
// See in Settings.h

// states to manually control CoWaS with buttons
bool ctrl_button;       // if possible to control CoWaS with buttons
enum ctrl_state {ctrl_pump, ctrl_motor_spool, ctrl_v23, ctrl_v1, ctrl_vman, ctrl_man_slot, ctrl_micro_pump};
uint8_t const ctrl_state_len = 7; // to update when modifying lenght of control states

String ctrl_state_name[ctrl_state_len] = {"Pump", "Spool", "V23", "V1", "Vman", "Man Slot", "Micro Pump"};   

ctrl_state control_state;


// ============= REAL HARDWARE =================
// ====> do not forget to add the object.begin() in setup()
C_output output;                       // custom print function to handle multiple outputs
Led status_led;                             // general purpose LED
Led green_led;                              // general purpose LED, used here for start signals
Trustability_ABP_Gage pressure1;            // see schematics
Valve_2_2 valve_1;                          // see schematics
Valve_3_2 valve_23;                         // see schematics, voltage off: L_way
Valve_2_2 valve_manifold;                   // see schematics
Pump pump;                                  // see schematics
Pump pump_vacuum;                           // see schematics
Micro_Pump micro_pump;                      // For DNA shield
Motor spool;                                // see schematics
Motor manifold_motor;                       // see schematics
Encoder encoder;                            // see schematics
Button button_start;                        // User button. Normally open
Button button_left;                         // User button. Normally open
Button button_right;                        // User button. Normally open
Button button_container;                    // Control button. Normally closed
Button button_spool_up;                     // Control button. Normally closed
Button button_spool_down;                   // Control button. Normally closed
struct Timer timer_control_pressure1;       // Timer for interrupts with pressure sensor 1
Manifold manifold;                          // Manifold

// new sensors
Flow_sensor flow_sensor_small;
Flow_sensor flow_sensor_big;

BigPressure pressure2;      

// PressureSensor pressureDFRobot;

void setup()
{

    // ========== SYSTEM INITIALIZATION ============
    SPI.begin();
    output.begin(terminal); // choose output of logs
    timer_control_pressure1 = {TC1, 0, TC3_IRQn, 4};    // timer of 1 second (4)

    // ========== HARDWARE INITIALIZATION ==========
    status_led.begin(STATUS_LED_PIN, "status");
    green_led.begin(GREEN_LED_PIN, "green");
    pressure1.begin(PRESSURE1_PIN, 3, "P1");
    valve_1.begin(VALVE_1_PIN, "V1");
    valve_23.begin(VALVE_23_PIN, "V_23");
    valve_manifold.begin(VALVE_MANIFOLD, "V_manifold");
    pump.begin(PUMP_PIN, true, "P1", PUMP_ENABLE);
    micro_pump.begin(ON_OFF_33V, "DNA Shield pump");
    spool.begin();
    manifold_motor.begin("MANIFOLD");
    encoder.begin(ENCODER_A_PIN, ENCODER_B_PIN, ENCODER_Z_PIN, 720, 10);
    button_left.begin(BUTTON_LEFT_PIN, "B_left");
    button_right.begin(BUTTON_RIGHT_PIN, "B_right");
    button_start.begin(BUTTON_START_PIN, "B_start");
    button_container.begin(BUTTON_CONTAINER_PIN, "B_container");
    button_spool_up.begin(BUTTON_SPOOL_UP, "B_spool_UP");
    // interrupt
    attachInterrupt(digitalPinToInterrupt(BUTTON_SPOOL_UP), ISR_emergency_stop_up, FALLING);
    button_spool_down.begin(BUTTON_SPOOL_DOWN, "B_spool_down");
    // ! doing problem right now, to investigate
    // attachInterrupt(digitalPinToInterrupt(BUTTON_SPOOL_DOWN), ISR_emergency_stop_down, FALLING);
    spool.endstop_up = false;
    spool.endstop_down = false;
    manifold.begin();
    manifold.change_state(PURGE_SLOT, unaivailable); // The purge has no filter

    // new sensors
    flow_sensor_small.begin(FLOW_SMALL_PIN, calibrationFactor_small, &pulseCount_small, pulseCounter_small);
    flow_sensor_big.begin(FLOW_BIG_PIN, calibrationFactor_big, &pulseCount_big, pulseCounter_big);

    pressure2.begin(pressure_2_pin);


    output.println("system initalized\n");

    // // ======== PRE-STARTING EXECUTION =========
    // output.println("========== Press start button to play program ==========================");
    // output.println("========== Press left button to move spool up ==========================");
    // output.println("========== Press right button to move spool down =======================");
    // output.println("========== Press reset button on due button to come back here ==========");
    // output.flush();

    green_led.on();
    status_led.on();
#if (DEBUG_MODE_PRINT)
    before_start_program();
    button_start.waitPressedAndReleased();
#endif
    green_led.off();
    status_led.off();

#ifdef SYSTEM_CHECKUP
    #if(DEBUG_MODE_PRINT)
        system_checkup();
        output.println("System checked\n");
    #endif
#endif

    output.println("Programm started\n");
    // System states are command for the server to allow or not some manipulations
    set_system_state(state_idle);

    // Communication with rapsberry
    Serial.begin(9600);

    ctrl_button = true;

    // ! calling testing function
    // purge_pipes_manifold();

    // ! Flow sensor test
    // flow_setup();
    // pressure2.readPressure();
    // delay(500);
    // pressure2.readPressure();

    pump_pid.setOutputLimits(0, 100);       // always do in setup/ or in pump.begin()

    pump_pid.setPID(200, 100, 100);

    test_all_components();
    // testing dna shield
    //rotateMotor(1);
    spool.start_origin();

    // fill_DNA_shield_tube();


    // all_on();
}

uint32_t last_p_p = millis();
float pressure_meas_count = 0;
float avg_old = 0;
float avg_new = 0;

void loop()
{
    // flow_loop();
    button_control();
    // SIC_control();
    // main_program();


    // float pressure;
    // float pressureBig;
    // avg_old += pressure1.getPressure();
    // avg_new += pressure2.readPressure();
    // pressure_meas_count = pressure_meas_count + 1.0;

    delay(50);
}

void main_program()
{
    if (Serial.available() > 0) {
        String data = Serial.readStringUntil('\n');
        int depth=0;
        String cmp_temp="sample";
        if (data.startsWith(cmp_temp)){
            if (data.length()==15){
                depth=data.substring(14,15).toInt();
            }
            else if (data.length()==16){
                depth=data.substring(14,16).toInt();
            }
            else{
                Serial.println("Error depth");
            }
            data="sampleFunction";
        }

        if(data == "reloadManifoldFunction"){
            manifold.reload();
        }
        else if(data == "purgeSterivexFunction"){
            purge_sterivex(PURGE_SLOT);
        }
        else if(data == "purgeContainerFunction"){
            step_purge();
        }
        else if(data == "purgePipesFunction"){
            purge_pipes_manifold();
        }
        else if(data == "fillContainerFunction"){
            step_fill_container();
        }
        else if(data == "sampleFunction"){
            depth*=100; // convert m -> cm
            sample_process(depth); 
        }
        else if(data == "rollingFunctionUp"){
            uint8_t speedy = 60;
            spool.set_speed(speedy, up);
            spool.start();
        }
        else if(data == "rollingFunctionDown"){
            uint8_t speedy = 60;
            spool.set_speed(speedy, down);
            spool.start();
        }
        else if(data == "stopRolling"){
            spool.stop();
        }
        else if(data == "startDNA"){
            micro_pump.start();
        }
        else if(data == "stopDNA"){
            micro_pump.stop();
        }

        Serial.print(" - function accomplished - ");
        Serial.println(data);
        Serial.flush();
    }
}


/*
    For valves: left close, right open
    Valves 23: left I way, right L way
    Motor spool: left up, right down, push and hold
    manifold: left, decrement slot and right increment
    Pump: right start, left stop
*/
int8_t pump_power = 20;
bool pump_on = false;

void button_control(){
    static uint8_t current_slot = 0;
    if (!ctrl_button){
        return;
    }
    
    switch (control_state){
        case ctrl_pump:{
            if (button_right.isPressed()) {
                if (pump_on == false){
                pump.set_power(pump_power);
                valve_manifold.set_open_way();
                pump.start();
                button_right.waitPressedAndReleased();
                }
                else{
                    pump.stop();
                    valve_manifold.set_close_way();
                    button_right.waitPressedAndReleased();
                }
                pump_on = !pump_on;
            }
            if (button_left.isPressed()){
                pump_power += 10;
                if (pump_power > 100){pump_power = 20;}
                if (pump_on){pump.set_power(pump_power);}
                button_left.waitPressedAndReleased();
                Serial.print("New pump power: ");
                Serial.println(pump_power);
            } 
            break;
        }  
        case ctrl_v23:{
            if (button_left.isPressed()){
                valve_23.set_L_way();
                button_left.waitPressedAndReleased();
            }
            if (button_right.isPressed()){
                valve_23.set_I_way();
                button_right.waitPressedAndReleased();
            }
            break;
        }
        case ctrl_v1: {
            if (button_left.isPressed()){
                valve_1.set_close_way();
                button_left.waitPressedAndReleased();
            }
            if (button_right.isPressed()){
                valve_1.set_open_way();
                button_right.waitPressedAndReleased();
            }
            break;
        }
        case ctrl_vman: {
            if (button_left.isPressed()){
                valve_manifold.set_close_way();
                button_left.waitPressedAndReleased();
            }
            if (button_right.isPressed()){
                valve_manifold.set_open_way();
                button_right.waitPressedAndReleased();
            }
            break;
        }
        case ctrl_man_slot: {
            if (button_right.isPressed()){
                current_slot++;
                current_slot = current_slot % 15;
                Serial.println(current_slot);
                button_right.waitPressedAndReleased();
                rotateMotor(current_slot);
            }
            if (button_left.isPressed()){
                current_slot--;
                if (current_slot > 14) current_slot = 14;
                Serial.println(current_slot);
                button_left.waitPressedAndReleased();
                rotateMotor(current_slot);
            }
            current_slot = current_slot % 15;
            break;
        }
        case ctrl_motor_spool: {
            if (button_right.isPressed() && button_spool_down.isReleased()){
                spool.set_speed(50, down);
                spool.start();
                button_right.waitPressedAndReleased();
                spool.stop();
            }
            if (button_left.isPressed() && button_spool_up.isReleased()){
                spool.set_speed(100, up);
                spool.start();
                button_left.waitPressedAndReleased();
                spool.stop();
            }
            break;
        }
        case ctrl_micro_pump: {
            if (button_right.isPressed()) {
                micro_pump.start();
                button_right.waitPressedAndReleased();
            }
            if (button_left.isPressed()){
                micro_pump.stop();
                button_left.waitPressedAndReleased();
            } 
            break;
        }
        default: break;
    }

    // check if going onto next mode
    if (button_start.isPressed()){
        control_state = static_cast<ctrl_state>((static_cast<uint8_t>(control_state) + 1) % ctrl_state_len);
        button_start.waitPressedAndReleased();
        Serial.print("New control state : ");
        Serial.print(ctrl_state_name[control_state]);
        Serial.print(" : ");
        Serial.println(control_state);
    }
}

void SIC_control(){
    if (button_right.isPressed()){
        button_right.waitPressedAndReleased();

        manifold.begin();
        manifold.change_state(PURGE_SLOT, unaivailable); // The purge has no filter

        demo_sample_process();

        green_led.off();
        status_led.off();
    }
    else if (button_left.isPressed()){
        button_left.waitPressedAndReleased();
        status_led.on();

        button_start.waitPressedAndReleased();

        green_led.on();

        step_dive(50);

        green_led.off();
        button_start.waitPressedAndReleased();
        green_led.on();
        spool.start_origin();

        green_led.off();
        status_led.off();
    }
    

}


