/**
 * @file Flow_sensor.cpp
 * @author Felix Schmeding
 * @brief For flowsensors from tinytronics
 * @version 0.1
 * @date 2024-12-16
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include <Arduino.h>
#include "Flow_sensor.h"


// ------------------------------ Interrupt routines, add one per flowsensor----------
volatile uint32_t pulseCount_small = 0;
volatile uint32_t pulseCount_big = 0;

void pulseCounter_small()
{
  // Increment the pulse counter
  pulseCount_small++;
}
void pulseCounter_big()
{
  // Increment the pulse counter
  pulseCount_big++;
}


/**
 * @brief Not implemented.
 * @param input_pin Input connection on the board
 * @param pulse_per_L calibration factor
 * @param pulseCount pointer to pulse counter to be able to read it
 * @param pulseCounter functin pointer of counting function
 */
void Flow_sensor::begin(byte input_pin, uint8_t cal_factor, volatile uint32_t* pulseCount, interruptRoutine pulseCounter)
{
    _pin = input_pin;
    pinMode(_pin, INPUT);

    _pulsePerL = cal_factor * 60;
    _pulseCount = pulseCount;
    _pulseCounter = pulseCounter;
    _lastUpdate = millis();

    //_flowMilliLitres = 0;
    _totalMilliLitres = 0;
    _flowRate = 0;

    activate(); // ? by default or not?
}

void Flow_sensor::activate()
{
    attachInterrupt(_pin, _pulseCounter, FALLING);
}

void Flow_sensor::deactivate()
{
    detachInterrupt(_pin);
}


void Flow_sensor::update(){
    deactivate(); // avoid messing up interupts
    uint32_t nb_pulses = *_pulseCount;
    *_pulseCount = 0;
    // once read and reset can continue to handle new pulses
    activate();
    // Serial.print("Number of pulses counted : ");
    // Serial.println(nb_pulses);
    float new_water = nb_pulses * 1000.0 / float(_pulsePerL); // [mL]

    // Serial.print("New water : ");
    // Serial.println(new_water);

    _totalMilliLitres += new_water;
    // 60000 to get from ms to mins and 1000 to get from mL to L
    _flowRate = new_water * 60000.0 / ((millis() - _lastUpdate) * 1000.0); // [L/min]

    _lastUpdate = millis();
}

float Flow_sensor::get_flowRate(){
    return _flowRate;
}

unsigned int Flow_sensor::get_totalFlowMilliL(){
    return _totalMilliLitres;
}

void Flow_sensor::reset_values(){
    _totalMilliLitres = 0;
    _flowRate = 0;
    *_pulseCount = 0;

    _lastUpdate = millis();
}

void Flow_sensor::print(){
    Serial.print("Flow rate : ");
    Serial.print(_flowRate);
    Serial.print("L/min");
    Serial.print("\t"); 		  // Print tab space

    // Print the cumulative total of litres flowed since starting
    Serial.print("Output Liquid Quantity: ");        
    Serial.print(_totalMilliLitres);
    Serial.print("mL"); 
    Serial.print("\t"); 		  // Print tab space
    Serial.print(_totalMilliLitres/1000.0);
    Serial.println("L");
}



// const uint8_t Pressure_1 = A1;    // pressure 0-16Mpa (water and air)
// const uint8_t Pressure_2 = A2;    // pressure 0-12 bar (cheaper one, only water)
// const uint8_t flow_small = 12;
// const uint8_t flow_big = 13;

// float calibrationFactor_small = 77.0;

// volatile byte pulseCount_small;  

// float flowRate_small;
// unsigned int flowMilliLitres_small;
// unsigned long totalMilliLitres_small;

// float calibrationFactor_big = 70.0;

// volatile byte pulseCount_big;  

// float flowRate_big;
// unsigned int flowMilliLitres_big;
// unsigned long totalMilliLitres_big;

// unsigned long oldTime;

// uint32_t pressure_val_1;
// uint32_t pressure_val_2;

// void pulseCounter_small();
// void pulseCounter_big();

// void read_print_pres();
// void print_flow();

// void flow_setup() {
//   // put your setup code here, to run once:
//   attachInterrupt(flow_small, pulseCounter_small, FALLING);
//   attachInterrupt(flow_big, pulseCounter_big, FALLING);

//   pinMode(Pressure_1, INPUT);   // then read with analogRead(pinnumber)
//   pinMode(Pressure_2, INPUT);
//   pinMode(flow_small, INPUT);
//   pinMode(flow_big, INPUT);

//   Serial.begin(9600);

// }

// void flow_loop() {
//   // put your main code here, to run repeatedly:
//   if((millis() - oldTime) > 300)    // Only process counters once per second
//     { 
//     oldTime = millis();


//     // print_flow();

//     // * PRESSURE
//     read_print_pres();

//   }
// }

// void read_print_pres(){
//   pressure_val_1 = analogRead(Pressure_1);
//   pressure_val_2 = analogRead(Pressure_2);

//   Serial.println("-------------------");
//   Serial.print("Pressure 1 is (analog value) : ");
//   Serial.println(pressure_val_1);
//   Serial.print("Pressure 2 is (analog value) : ");
//   Serial.println(pressure_val_2);
//   Serial.println("-------------------");
// }


// void pulseCounter_small()
// {
//   // Increment the pulse counter
//   pulseCount_small++;
// }
// void pulseCounter_big()
// {
//   // Increment the pulse counter
//   pulseCount_big++;
// }

// void print_flow(){
//     detachInterrupt(flow_small);
//     detachInterrupt(flow_big);

//     flowRate_small = ((1000.0 / (millis() - oldTime)) * pulseCount_small) / calibrationFactor_small;
//     flowRate_big = ((1000.0 / (millis() - oldTime)) * pulseCount_big) / calibrationFactor_big;

//     // * SMALLLL --------------------

//     flowMilliLitres_small = (flowRate_small / 60) * 1000;
    
//     // Add the millilitres passed in this second to the cumulative total
//     totalMilliLitres_small += flowMilliLitres_small;
    
//     // Print the flow rate for this second in litres / minute
//     Serial.print("Flow rate small : ");
//     Serial.print(flowRate_small);  // Print the integer part of the variable
//     Serial.print("L/min");
//     Serial.print("\t"); 		  // Print tab space

//     // Print the cumulative total of litres flowed since starting
//     Serial.print("Output Liquid Quantity: ");        
//     Serial.print(totalMilliLitres_small);
//     Serial.print("mL"); 
//     Serial.print("\t"); 		  // Print tab space
//     Serial.print(totalMilliLitres_small/1000.0);
//     Serial.println("L");


//     // * BIIIIG -----------------------

//     flowMilliLitres_big = (flowRate_big / 60.0) * 1000;
    
//     // Add the millilitres passed in this second to the cumulative total
//     totalMilliLitres_big += flowMilliLitres_big;
    
//     // Print the flow rate for this second in litres / minute
//     Serial.print("Flow rate big: ");
//     Serial.print(flowRate_big);  // Print the integer part of the variable
//     Serial.print("L/min");
//     Serial.print("\t"); 		  // Print tab space

//     // Print the cumulative total of litres flowed since starting
//     Serial.print("Output Liquid Quantity: ");        
//     Serial.print(totalMilliLitres_big);
//     Serial.print("mL"); 
//     Serial.print("\t"); 		  // Print tab space
//     Serial.print(totalMilliLitres_big/1000.0);
//     Serial.println("L");

//     pulseCount_small = 0;
//     pulseCount_big = 0;

//     attachInterrupt(flow_small, pulseCounter_small, FALLING);
//     attachInterrupt(flow_big, pulseCounter_big, FALLING);
// }

