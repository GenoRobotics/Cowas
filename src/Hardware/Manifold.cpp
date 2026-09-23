#include <Arduino.h>
#include "Manifold.h"
#include "Button.h"
#include "C_output.h"
#include <SPI.h>

extern C_output output;
extern Motor manifold_motor;
extern Button button_start;
//----- MOTOR -----//
const uint16_t speed = 40; // Refers to power, min 70 to move

// ! move back to .h
//float angle_offset_neg = -1.05;   // when turning CCW
float angle_offset_pos = 5.2;   // when turning CW

motor_direction direction = down; // 1 or -1
int nb_turns = 0;
float current_angle;
float deg_encoderPosition = 0;// encoder position in degrees
float deg_previousEncoderPosition = 0;

// first element is the purge angle. The following elements are the sterivex angles in the order we want them to be used.
float sterivex_angle[15];
// raw slot index (0-15, see PURGE_RAW_OFFSET/omitted_angle_nb in Manifold.h) each
// sterivex_angle[]/human_idx entry was built from - kept alongside it so
// clock_minutes_for_slot() doesn't have to redo the walk in Manifold::begin().
int raw_slot_of_human_idx[15];

/**
 * @brief Constructor for the manifold slots
 * @param pin_control Output connection on the board of the slot
 * @param _ID id of the manifold slot
 */
void Manifold_slot::begin(byte _pin_control, int _id)
{
    ID = _id;
    state = available;
    pin_control = _pin_control;
    // pinMode(pin_control, OUTPUT); // Assigned a pin to each slot
    if (VERBOSE_INIT){output.println("Slot manifold " + String(ID) + " initiated");}
}

int Manifold_slot::get_id()
{
    return ID;
}

void Manifold_slot::change_state(slot_state changed_state)
{
    state=changed_state;
}

slot_state Manifold_slot::get_state()
{
    return state;
}


/**
 * @brief Constructor for the manifold
 */
void Manifold::begin()
{
    for(int i=0; i < NB_SLOT; i++){
        slots[i].begin(0, i);     // TODO: remove pin, not used
    }   

    pinMode(ENCODER_MANIFOLD, OUTPUT);
    digitalWrite(ENCODER_MANIFOLD, HIGH);

    // Creation of array with sterivex position in angle.
    // Walks outward from the purge position (PURGE_RAW_OFFSET raw steps from the
    // calibrated reference), so index 0 = purge and 1..14 = samples, skipping the
    // no-hole position (omitted_angle_nb) wherever it falls along that walk.
    int human_idx = 0;
    for (int offset = 0; offset < 16; offset++){
      int slot = (PURGE_RAW_OFFSET + offset) % 16;
      if (slot == omitted_angle_nb){
        continue; // no hole at this physical position
      }
      float angle = purge_angle - slot*angle_between_slots;
      if (angle < 0){
        angle += 360.0;
      }
      sterivex_angle[human_idx] = angle;
      raw_slot_of_human_idx[human_idx] = slot;
      human_idx++;
    }

    if (VERBOSE_INIT){output.println("Manifold initiated");}
}

int Manifold::get_id(int i)
{
    return slots[i].get_id();
}

void Manifold::change_state(int i, slot_state changed_state)
{
    slots[i].change_state(changed_state);
}

slot_state Manifold::get_state(int i)
{
    return slots[i].get_state();
}

void Manifold::reload()
{
    for(int i=0; i < NB_SLOT; i++){
          slots[i].change_state(available);
      }   
}

bool rotateMotor(int index)
{
  // if the manifold is not connected to the system, exit function
  if(MANIFOLD_USE == false){
    return true;
  }

  // SETUP//
  bool end_rotation = false;
  nb_turns = 0; //counts number of turns in case the position is <0deg or >360deg.
  float angle_to_reach = sterivex_angle[index];

  readEncoder(true);
  if (!readEncoder(false))
  {
    output.println("ERROR | rotateMotor: could not get a starting position from the encoder - aborting, motor never started");
    return false;
  }
  directionDetermination(angle_to_reach);

  manifold_motor.start(speed, direction);
  if (VERBOSE_MANIFOLD){
    output.print("Motor started with direction ");
    output.println(direction == down ? "DOWN (CW)" : "UP (CCW)");
  }

  uint32_t last_print = 0;
  uint32_t rotation_start = millis();
  uint8_t consecutive_failures = 0;

  float scaled_goal = scale_angle(angle_to_reach);
  float adapted_goal_angle; // incorporates slip of motor, need to calibrate manually

  if (direction == down) {
      adapted_goal_angle = scaled_goal - angle_offset_pos;
  }
  else if (direction == up){
      adapted_goal_angle = scaled_goal + angle_offset_neg;
  }
  // should never overflow

  while (end_rotation == false)
  {
    consecutive_failures = readEncoder(false) ? 0 : (consecutive_failures + 1);

    // Safety net: without this, a persistent encoder failure (current_angle frozen,
    // so the stop condition below is never re-evaluated with fresh data) or a real
    // mechanical stall used to spin this loop forever with the motor left running
    // and zero feedback - see MANIFOLD_ENCODER_MAX_CONSECUTIVE_FAILURES/
    // MANIFOLD_ROTATE_TIMEOUT_MS (Settings.h).
    if (consecutive_failures >= MANIFOLD_ENCODER_MAX_CONSECUTIVE_FAILURES)
    {
      manifold_motor.stop();
      output.println("ERROR | rotateMotor: aborted - lost the encoder (too many consecutive bad reads). Motor stopped, position may be off - re-home/recalibrate before trusting slot positions.");
      return false;
    }
    if (millis() - rotation_start > MANIFOLD_ROTATE_TIMEOUT_MS)
    {
      manifold_motor.stop();
      output.println("ERROR | rotateMotor: aborted - timed out before reaching the target (possible mechanical stall). Motor stopped.");
      return false;
    }

    if(VERBOSE_MANIFOLD && (millis() - last_print > 500)){
      output.print("   Angle to reach: ");
      output.println(angle_to_reach);
      last_print = millis();
    }

    // checking if position passed threshold (adapted_goal_angle)
    if ((direction == down && (scale_angle(current_angle) >= adapted_goal_angle)) ||
        (direction == up && (scale_angle(current_angle) <= adapted_goal_angle)))
    {
      manifold_motor.stop();
      end_rotation = true;
    }

    delay(1);//smaller delay -> better precision
  }

  return true;
}

/**
 * @brief Human slot index (0=purge, 1-14=samples) -> clock-face minute label
 * (0-59), top of the manifold = 0. 16 raw positions around the circle * 3.75
 * min/position = 60 min, same scale as a real clock face. See
 * MANIFOLD_RAW_INDEX_INCREASES_CW (Settings.h) for the physical-direction caveat.
 */
int clock_minutes_for_slot(int human_slot)
{
  if (human_slot < 0 || human_slot >= NB_SLOT){
    return -1;
  }

  int raw = raw_slot_of_human_idx[human_slot];
  float minutes = raw * (60.0 / 16.0);
  if (!MANIFOLD_RAW_INDEX_INCREASES_CW){
    minutes = 60.0 - minutes;
  }

  int rounded = (int)lround(minutes) % 60;
  if (rounded < 0){
    rounded += 60;
  }
  return rounded;
}

/// @brief Inverse of clock_minutes_for_slot() - the human slot at a given clock
/// minute label, or -1 if none matches exactly.
int slot_for_clock_minutes(int minutes)
{
  for (int human_slot = 0; human_slot < NB_SLOT; human_slot++){
    if (clock_minutes_for_slot(human_slot) == minutes){
      return human_slot;
    }
  }
  return -1;
}

void directionDetermination(float goal_angle)
{
  float diff_angle;
  diff_angle = scale_angle(goal_angle) - scale_angle(current_angle);

  // For debug
  // output.print("Scaled angle goal : ");
  // output.println(scale_angle(goal_angle));
  // output.print("Scaled current angle : ");
  // output.println(scale_angle(current_angle));
  // output.print("Difference : ");
  // output.println(diff_angle);

  // if diff >= 0 then down (CW), else going up (CCW)
  direction = diff_angle >= 0 ? down : up;
}


bool readEncoder(bool init_setup)
{
  static uint32_t last_print = millis();
  static uint32_t last_error_print = 0;

  uint16_t encoderPosition; //a 16 bit variable to hold the encoders position (goes up to 4096)
  uint8_t attempts; //count how many times we've tried to obtain the position in case there are errors

  deg_previousEncoderPosition = deg_encoderPosition;

  //if you want to set the zero position before beggining uncomment the following function call
  // setZeroSPI(ENCODER_MANIFOLD);

  //set attemps counter at 0 so we can try again if we get bad position
  attempts = 0;

  //this function gets the encoder position and returns it as a uint16_t
  //send the function either res12 or res14 for your encoders resolution
  encoderPosition = getPositionSPI(ENCODER_MANIFOLD, RES12);

  //if the position returned was 0xFFFF we know that there was an error calculating the checksum
  //make 3 attempts for position. we will pre-increment attempts because we'll use the number later and want an accurate count
  while (encoderPosition == 0xFFFF && ++attempts < 3)
  {
    encoderPosition = getPositionSPI(ENCODER_MANIFOLD, RES12); //try again
  }

  if (encoderPosition == 0xFFFF) //position is bad, let the user know how many times we tried
  {
    // Always surfaced (not gated behind VERBOSE_MANIFOLD, unlike the routine print
    // below) - a communication failure here used to be completely silent, which is
    // exactly why "the manifold just stops moving" looked like a mystery: current_angle
    // freezes below without ANY indication why. Throttled since this can be polled
    // every ~1ms from rotateMotor()'s wait loop.
    if (millis() - last_error_print > 1000)
    {
      output.println("ERROR | manifold encoder: bad checksum after " + String(attempts) + " attempts - check AMT22 wiring/SPI bus");
      last_error_print = millis();
    }
    return false; // current_angle intentionally left untouched - caller must not trust it moved
  }

  //position was good
  deg_encoderPosition = encoderPosition * encoder_to_deg;

  if (!init_setup)
  {
    //increments or decrements nb_turns, if the 0/360deg point have been exceeded
    if ((deg_encoderPosition > 350) && (deg_previousEncoderPosition < 10))
      nb_turns = nb_turns - 1;
    else if ((deg_encoderPosition < 10) && (deg_previousEncoderPosition > 350))
      nb_turns = nb_turns + 1;
    current_angle = deg_encoderPosition + 360 * nb_turns;
  }

  if(VERBOSE_MANIFOLD && (millis() - last_print > 500)){
      output.print("Encoder: ");
      output.println(current_angle);

      last_print = millis();
    }

  return true;
}

/**
   This function gets the absolute position from the AMT22 encoder using the SPI bus. The AMT22 position includes 2 checkbits to use
   for position verification. Both 12-bit and 14-bit encoders transfer position via two bytes, giving 16-bits regardless of resolution.
   For 12-bit encoders the position is left-shifted two bits, leaving the right two bits as zeros. This gives the impression that the encoder
   is actually sending 14-bits, when it is actually sending 12-bit values, where every number is multiplied by 4.
   This function takes the pin number of the desired device as an input
   This funciton expects res12 or res14 to properly format position responses.
   Error values are returned as 0xFFFF
   @note: function from Sparkfun library
*/
uint16_t getPositionSPI(uint8_t encoder, uint8_t resolution)
{
  uint16_t currentPosition;       //16-bit response from encoder
  bool binaryArray[16];           //after receiving the position we will populate this array and use it for calculating the checksum

  //get first byte which is the high byte, shift it 8 bits. don't release line for the first byte
  currentPosition = spiWriteRead(AMT22_NOP, encoder, false) << 8;

  //this is the time required between bytes as specified in the datasheet.
  //We will implement that time delay here, however the arduino is not the fastest device so the delay
  //is likely inherantly there already
  delayMicroseconds(3);

  //OR the low byte with the currentPosition variable. release line after second byte
  currentPosition |= spiWriteRead(AMT22_NOP, encoder, true);

  //run through the 16 bits of position and put each bit into a slot in the array so we can do the checksum calculation
  for (int i = 0; i < 16; i++) binaryArray[i] = (0x01) & (currentPosition >> (i));

  //using the equation on the datasheet we can calculate the checksums and then make sure they match what the encoder sent
  if ((binaryArray[15] == !(binaryArray[13] ^ binaryArray[11] ^ binaryArray[9] ^ binaryArray[7] ^ binaryArray[5] ^ binaryArray[3] ^ binaryArray[1]))
      && (binaryArray[14] == !(binaryArray[12] ^ binaryArray[10] ^ binaryArray[8] ^ binaryArray[6] ^ binaryArray[4] ^ binaryArray[2] ^ binaryArray[0])))
  {
    //we got back a good position, so just mask away the checkbits
    currentPosition &= 0x3FFF;
  }
  else
  {
    currentPosition = 0xFFFF; //bad position
  }


  //If the resolution is 12-bits, and wasn't 0xFFFF, then shift position, otherwise do nothing
  if ((resolution == RES12) && (currentPosition != 0xFFFF)) currentPosition = currentPosition >> 2;

  return currentPosition;
}

/** 
   This function sets the state of the SPI line. It isn't necessary but makes the code more readable than having digitalWrite everywhere
   This function takes the pin number of the desired device as an input
  @note: function from Sparkfun library
*/
void setCSLine (uint8_t encoder, uint8_t csLine)
{
  digitalWrite(encoder, csLine);
}


/**
   This function does the SPI transfer. sendByte is the byte to transmit.
   Use releaseLine to let the spiWriteRead function know if it should release
   the chip select line after transfer.
   This function takes the pin number of the desired device as an input
   The received data is returned.
  @note: function from Sparkfun library
*/
uint8_t spiWriteRead(uint8_t sendByte, uint8_t encoder, uint8_t releaseLine)
{
  //holder for the received over SPI
  uint8_t data;

  //set cs low, cs may already be low but there's no issue calling it again except for extra time
  setCSLine(encoder , LOW);

  //There is a minimum time requirement after CS goes low before data can be clocked out of the encoder.
  //We will implement that time delay here, however the arduino is not the fastest device so the delay
  //is likely inherantly there already
  delayMicroseconds(3);

  //send the command
  data = SPI.transfer(sendByte);
  delayMicroseconds(3); //There is also a minimum time after clocking that CS should remain asserted before we release it
  setCSLine(encoder, releaseLine); //if releaseLine is high set it high else it stays low

  return data;
}

/**
 * @brief Calibrate the encoder of the manifold
 * TODO: implement calibration with button and integrate to GUI
 * @param speed Speed of the motor - kept deliberately slow (see MANIFOLD_CAL_SPEED)
 * so there's time to react and stop the motor exactly aligned.
 */
void calibrateEncoder(uint16_t speed){
    output.println("Calibrating encoder of the manifold");
    output.println("Be ready to press a key when the motor is aligned with slot 0");
    output.println("Send 'c' to rotate clockwise, 'x' to abort, or any other key to rotate anticlockwise");
    output.println("(pick whichever direction is safe from the manifold's current position)");

    while (!Serial.available())
    {
      delay(10);
    }

    char dir_choice = Serial.read();
    while (Serial.available())
    {
      Serial.read();
    }
    if (dir_choice == 'x' || dir_choice == 'X')
    {
      output.println("Aborted before starting.");
      return;
    }
    motor_direction dir = (dir_choice == 'c' || dir_choice == 'C') ? down : up; // down = clockwise, up = anticlockwise (see Motor.h)

    output.println(dir == down ? "Starting to rotate CLOCKWISE" : "Starting to rotate ANTICLOCKWISE");
    delay(1000);

    manifold_motor.start(speed, dir);
    
    while (!Serial.available()){
        delay(1);
    }

    manifold_motor.stop();

    while (Serial.available()){
        Serial.read(); // consume the stopping keypress - leaving it buffered would get eaten by whatever reads Serial next
    }

    // read encoder value and convert to degrees
    float pos = getPositionSPI(ENCODER_MANIFOLD, RES12);
    Serial.print("Manifold encoder value : ");
    Serial.println(pos);
    
    output.println("Please replace the purge angle of manifold by : ");
    output.println(pos * encoder_to_deg);
}

void setZeroSPI(uint8_t encoder)
{
  spiWriteRead(AMT22_NOP, encoder, false);

  //this is the time required between bytes as specified in the datasheet.
  //We will implement that time delay here, however the arduino is not the fastest device so the delay
  //is likely inherantly there already
  delayMicroseconds(3); 
  
  spiWriteRead(AMT22_ZERO, encoder, true);
  delay(250); //250 second delay to allow the encoder to reset
}

uint16_t getRotationSPI(uint8_t encoder){
    //read the two bytes for position from the encoder, starting with the high byte
    uint16_t encoderPosition = spiWriteRead(AMT22_NOP, encoder, false) << 8; //shift up 8 bits because this is the high byte
    delayMicroseconds(3);
    encoderPosition |= spiWriteRead(AMT22_TURNS, encoder, false); //we send the turns command (0xA0) here, to tell the encoder to send us the turns count after the position

    //wait 40us before reading the turns counter
    delayMicroseconds(40);

    //read the two bytes for turns from the encoder, starting with the high byte
    uint16_t encoderTurns = spiWriteRead(AMT22_NOP, encoder, false) << 8; //shift up 8 bits because this is the high byte
    delayMicroseconds(3);

    // releasing the line
    encoderTurns |= spiWriteRead(AMT22_NOP, encoder, true);
    delayMicroseconds(3);

    encoderPosition &= 0x3FFF;
    encoderTurns &= 0x3FFF;

    Serial.print("Manifold encoder turns : ");
    Serial.println(encoderTurns);
    Serial.print("With angle value : ");
    Serial.println(encoderPosition * encoder_to_deg);
    return encoderTurns;
}

float scale_angle(float angle){
  float new_angle;

  new_angle = angle - purge_angle + 11.25;

  uint8_t loop_nb = 0;
  while ((new_angle < -180 || new_angle >= 180) && loop_nb < 3){
    if (new_angle < -180){
      new_angle += 360.0;
    }
    else {
      new_angle -= 360.0;
    }
  }

  return new_angle;
}
