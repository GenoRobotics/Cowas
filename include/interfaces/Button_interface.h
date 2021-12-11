#ifndef BUTTON_INTERFACE_H
#define BUTTON_INTERFACE_H

#include <Arduino.h>
#include "Serial_output.h"


// =================== NOT WORKING

extern Serial_output output;

class Button_interface
{
protected:
    byte input_pin;
    int state;

public:
    void begin(byte _input_pin)
    {
        input_pin = _input_pin;
        update();
    }

    void update()
    {
        // You can handle the debounce of the button directly
        // in the class, so you don't have to think about it
        // elsewhere in your code
        output.print("Enter button state : ");
        if (output.get_serial_type() == terminal)
        {
            state = output.read();
            if (state > 1)
                state = 1;
        }
        output.println(state);
    }

    byte getState()
    {
        update();
        return state;
    }

    bool isPressed()
    {
        return (getState() == HIGH);
    }

    void waitPressedAndReleased()
    {
        output.print("Button pressed..."); 
        delay(500);
        output.println("and released");
    }
};

#endif