#include "Button.h"
#include "C_output.h"

extern C_output output;

/**
 * @brief Constructor for a button
 * @param _input_pin Input connection on the board
 *
 */
void Button::begin(byte _input_pin)
{
    input_pin = _input_pin;
    pinMode(input_pin, INPUT);
    update();
    output.println("Button " + ID + " initiated");
}
void Button::begin(byte _input_pin, String _ID)
{
    ID = _ID;
    begin(_input_pin);
}

/**
 * @brief udpate button state. Debounce incorporated.
 * 
 */
void Button::update()
{
    bool looping = true;
    while (looping)
    {
        byte newReading = digitalRead(input_pin);

        if (newReading != lastReading)
        {
            lastDebounceTime = millis();
        }
        if (millis() - lastDebounceTime > debounceDelay)
        {
            // Update the 'state' attribute only if debounce is checked
            state = newReading;
            looping = false;
        }
        lastReading = newReading;
    }
}

byte Button::getState()
{
    update();
    return state;
}

bool Button::isPressed()
{
    return (getState() == LOW);     // depends on how the circuit is
}

bool Button::isReleased()
{
    return (getState() == HIGH);    // depends on how the circuit is
}

/**
 * @brief blocks program until button is pressed and released
 * @note If button is malfuntioning, program will be blocked
 */
void Button::waitPressedAndReleased()
{
    while (!isPressed())
    {
        delay(5);
    }
    while (isPressed())
    {
        delay(5);
    }
}

/**
 * @brief blocks program until button is released and pressed
 * @note If button is malfuntioning, program will be blocked
 */
void Button::waitReleasedAndPressed()
{
    while (isPressed())
    {
        delay(5);
    }
    while (!isPressed())
    {
        delay(5);
    }
}