/* *************************************************************************** *
 * Project Title: RC car
 * 
 * Description: A small Arduino project that brings old and broken Nikko
 * VaporizR 2 car back to life. We use new motors, new motor driver board, 
 * an iA6C receiver module and a power step-down module to make use of the 
 * structure of the car useful again.
 *
 * Licence: GPL 3.0       (details in project file "LICENCE")
 * Version: 0.1.0         (details in project file "README.md")
 * created: 14. 04. 2024
 * by:      Luka Oman
 * *************************************************************************** */

// Declare additional libraries
#include <ppm.h>

// Define Arduino pins
static const int receiverPin = 3;

// Define new objects

// Define constants
#define channelSafeMode 6        // kill switch
#define channelLeftRight 1       // turn left or right
#define channelForwardReverse 2  // go forward or reverse

// in theory the signal can go from 1000 (low) to 1500 (neutral) and from 1500 (neutral) to 2000 (high)
// but in reallity those values can vary so we use user defined constants
static const int lowNegative = 980;    // lowest signal received
static const int highNegative = 1450;  // low breaking point for neutral signal
static const int lowPositive = 1550;   // high breaking point for neutral signal
static const int highPositive = 2020;  // highest signal received

// Define variables
bool carOn = false;  // kill switch
/* We do not need this part of code (yet)
bool motorAForward = true;  // determinate motor A direction (not to burn the motor driver)
bool motorBForward = true;  // determinate motor B direction (not to burn the motor driver)
*/

int throttleIndex = 0;    // 0% => stop, 100% => full power
int motorDirection = 0;  // -1 => reverse, 0 => stop, 1 => forward

// Define custom functions

// Initialization function, run only once
void setup() {
  Serial.begin(9600);
  ppm.begin(receiverPin, false);
}

// Main function, run in a loop
void loop() {

  // get channels values
  short getSafeMode = ppm.read_channel(channelSafeMode);
  short getLeftRight = ppm.read_channel(channelLeftRight);
  short getForwardReverse = ppm.read_channel(channelForwardReverse);

  // determinate limits, so the misconfigured receiver can not break code
  carOn = (getSafeMode < lowPositive) ? false : true;

  getLeftRight = (getLeftRight < lowNegative) ? lowNegative : getLeftRight;
  getLeftRight = (getLeftRight > highPositive) ? highPositive : getLeftRight;

  getForwardReverse = (getForwardReverse < lowNegative) ? lowNegative : getForwardReverse;
  getForwardReverse = (getForwardReverse > highPositive) ? highPositive : getForwardReverse;

  // Write current state of the received values => simple parsing
  if (carOn == true) {
    Serial.print("Car is ON\t|");
  } else {
    Serial.print("Car is OFF\t|");
  }

  if (getLeftRight < highNegative) {
    Serial.print(" Left   |");
  } else if (getLeftRight > lowPositive) {
    Serial.print(" Right  |");
  } else {
    Serial.print(" Center |");
  }

  /* ***** D E P R E C A T E D - C O D E *****
  if (getForwardReverse < highNegative) {
    Serial.print(" Reverse |");
  } else if (getForwardReverse > lowPositive) {
    Serial.print(" Forward |");
  } else {
    Serial.print(" Stop    |");
  }
*/

  // real parsing of the code, used to move the car in the desired direction
  if (carOn == true) {

    // get the throttle index, that is the persentage of throttle the driver behing the TX want
    if ((motorDirection < 0 && getForwardReverse >= highNegative) || (motorDirection > 0 && getForwardReverse <= lowPositive)) {
      // STOP => we are changing direction and would not want to overload the motor driver
      motorDirection = 5;  // <= has to be "0", we are using other number just to debug this state
      throttleIndex = 0;
      delay(1000);  // <= has to be "500" milliseconds, we are using higher delay just to debug this state
    } else if (getForwardReverse < lowPositive && getForwardReverse > highNegative) {
      // STOP => the TX stick is in neutral position
      motorDirection = 0;
      throttleIndex = 0;
    } else if (getForwardReverse >= lowPositive) {
      // FORWARD
      throttleIndex = map(getForwardReverse, lowPositive, highPositive, 0, 100);
      motorDirection = 1;
    } else if (getForwardReverse <= highNegative) {
      // REVERSED
      throttleIndex = map(getForwardReverse, highNegative, lowNegative, 0, 100);
      motorDirection = -1;
    }

    (motorDirection >= 0) ? Serial.print("  ") : Serial.print(" ");
    Serial.print(motorDirection);
    Serial.print(" | ");
    Serial.print(throttleIndex);
    Serial.print(" | ");
  }





















  Serial.println();
  delay(10);
}