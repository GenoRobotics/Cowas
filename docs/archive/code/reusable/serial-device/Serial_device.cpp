/**
 * @file Serial_device.cpp
 * @author Timothée Hirt & Paco Mermoud
 * @brief Class to intiatlize communication with another device on Serial lines.
 *        Here for Wi-Fi card
 * @version 0.1
 * @date 2022-01-29
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "Serial_device.h"
#include "C_output.h"
#include "Settings.h"
#include "GPIO.h"

extern C_output output;
extern GPIO wifi_message;

void Serial_device::begin(){
    Serial1.begin(115200);
    delay(100);
    output.println("Serial with device initialised");
}

void Serial_device::start_communication(){
    // wait message line to be pulled high
    while(wifi_message.read() == 0){
        delay(500);
    }
    
    // allows to delete any data in the buffer
    while(Serial1.available() > 0){
        Serial1.read();
    }
    Serial1.flush();

    // Why not.
    Serial1.print("1234");
}

/**
 * @brief Receive the time under the epoch format (seconds from 1970)
 * 
 * @return struct Date 
 */
struct Date Serial_device::receive_time(){

    struct Date date;

    while(Serial1.available() == 0);
    Serial1.setTimeout(2000);
    date.epoch = (time_t) Serial1.parseInt();
    output.println((uint32_t)date.epoch);
    output.flush();
    return date;
    
}

void Serial_device::validate(){
    Serial1.print("1");
    while(Serial.available() == 0);
    if(Serial.parseInt() != 1){
        output.println("Failed serial1 validation");
    }
}

String Serial_device::receive(){
    return Serial1.readString();
}

int Serial_device::available(){
    return Serial1.available();
}

void Serial_device::flush(){
    Serial1.flush();
}

long Serial_device::parseNumber(){
    return Serial1.parseInt();
}