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

// Define variables
bool carOn = false;         // kill switch
bool motorAForward = true;  // determinate motor A direction (not to burn the motor driver)
bool motorBForward = true;  // determinate motor B direction (not to burn the motor driver)

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

  if (getSafeMode < 1600) {
    carOn = false;
  } else {
    carOn = true;
  }

  if (carOn == true) 
    Serial.print("Car is ON\t|");
  } else {
    Serial.print("Car is OFF\t|");
  }

  if (getLeftRight < 1400) {
    Serial.print(" Left\t|");
  } else if (getLeftRight > 1600) {
    Serial.print(" Right\t|");
  } else {
    Serial.print(" Center\t|");
  }

  if (getForwardReverse < 1400) {
    Serial.print(" Reverse\t|");
  } else if (getForwardReverse > 1600) {
    Serial.print(" Forward\t|");
  } else {
    Serial.print(" Stop\t|");
  }

  Serial.println();
  delay(50);
}