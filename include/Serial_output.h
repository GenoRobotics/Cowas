#ifndef SERIAL_OUTPUT_H
#define SERIAL_OUTPUT_H

using namespace std;

#include <Arduino.h>

enum serial_type_enum{terminal, SD};

class Serial_output
{

private:
    serial_type_enum serial_type;

public:
    // Serial_output(uint8_t _serial_type);
    // template<typename T>
    // void print(T output, bool line_break);
    // template<typename T>
    // void Serial_output::print(T output);
    // template<typename T>
    // void Serial_output::println(T output);

/**
 * @brief Choose output of arduino
 * @param _serial_type 1 is serial monitor, 2 is sd card
 *
 */
void begin(serial_type_enum _serial_type){
    serial_type = _serial_type;
    digitalWrite(LED_BUILTIN, HIGH);
    switch(serial_type){
        case terminal: // serial monitor
            Serial.begin(9600);
            delay(100);
            Serial.println("Serial initialized");
            break;
        default:
            Serial.begin(9600);
            Serial.println("Error initalizing serial");
        case SD:
            break;
    }
}

template<typename T>
void print(T output)
{
    switch(serial_type){
        case terminal:
            Serial.print(output);
            break;
        case SD:
            break;
    }
}

template<typename T>
void println(T output)
{
    switch(serial_type){
        case terminal:
            Serial.println(output);
            break;
        case SD:
            break;
    }
}

int read(){
    int data;
    switch(serial_type){
        case terminal:
            data = Serial.read();
            if (data >= 48 && data <= 57) data = data-48;
            else data = 0;
            break;
        case SD:
            data = 0;
            break;
        default:
            data = 0;
    }
    return data;
}

int available(){
    int data;
    switch(serial_type){
        case terminal:
            data = Serial.available();
            break;
        case SD:
            break;
        default:
            data = 0;
    }
    return data;
}

serial_type_enum get_serial_type(){
    return serial_type;
}




};

#endif