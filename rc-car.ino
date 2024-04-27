/* *************************************************************************** *
 * Project Title: RC car
 * 
 * Description: A small Arduino project that brings old and broken Nikko
 * VaporizR 2 car back to life. We use new motors, new motor driver board, 
 * an iA6C receiver module and a power step-down module to make use of the 
 * structure of the car useful again.
 *
 * Licence:  GPL 3.0       (details in project file "LICENCE")
 * Version:  0.3.4         (details in project file "README.md")
 * created:  14. 04. 2024
 * modified: 27. 04. 2024
 * by:       Luka Oman
 * *************************************************************************** */

// Declare additional libraries
#include <ppm.h>

// Define debuging messages
#define DebugOn  //comment to not run debug

# ifndef DebugOn
#   define DebugBegin(x)
#   define DebugPrint(x)
#   define DebugPrintln(x)
# else
#   define DebugBegin(x) Serial.begin(x)
#   define DebugPrint(x) Serial.print(x)
#   define DebugPrintln(x) Serial.println(x)
# endif


// Define Arduino pins
static const int receiverPin = 3;
static const int motorA1Pin = 5;
static const int motorA2Pin = 6;
static const int motorB1Pin = 10;
static const int motorB2Pin = 11;

// Define new objects

// Define constants
#define channelSafeMode 5        // kill switch
#define channelSpeedLevel 6      // three position switch for controling maximum speed
#define channelLeftRight 1       // turn left or right
#define channelForwardReverse 2  // go forward or reverse

// in theory the signal can go from 1000 (low) to 1500 (neutral) and from 1500 (neutral) to 2000 (high)
// but in reallity those values can vary so we use user defined constants
static const int lowNegative = 1050;   // lowest signal received
static const int highNegative = 1400;  // low breaking point for neutral signal
static const int lowPositive = 1600;   // high breaking point for neutral signal
static const int highPositive = 1950;  // highest signal received

// Define variables
bool carOn = false;  // kill switch

int throttleIndex = 0;   // 0% => stop, 100% => full power
int motorDirection = 0;  // -1 => reverse, 0 => stop, 1 => forward
int motorAIndex = 0;     // left  motor index, 0% => stop, 100% full power
int motorBIndex = 0;     // right motor index, 0% => stop, 100% full power

// Define custom functions
void runMotorA(int throttle, int direction) {

  // throttle is general, does not know if it is forward or reversed,
  // we need direction to correctly turn the motor
  if (direction == 1) {
    // FORWARD
    analogWrite(motorA1Pin, throttle);
    analogWrite(motorA2Pin, 0);
  } else if (direction == -1) {
    // REVERSE
    analogWrite(motorA1Pin, 0);
    analogWrite(motorA2Pin, throttle);
  }
}

void runMotorB(int throttle, int direction) {

  // throttle is general, does not know if it is forward or reversed,
  // we need direction to correctly turn the motor
  if (direction == 1) {
    // FORWARD
    analogWrite(motorB1Pin, throttle);
    analogWrite(motorB2Pin, 0);
  } else if (direction == -1) {
    // REVERSE
    analogWrite(motorB1Pin, 0);
    analogWrite(motorB2Pin, throttle);
  }
}

void stopBothMotors(bool motorBreak) {

  // true if break (both HIGH), false is stop (both LOW)
  if (motorBreak == true) {
    digitalWrite(motorA1Pin, HIGH);
    digitalWrite(motorA2Pin, HIGH);
    digitalWrite(motorB1Pin, HIGH);
    digitalWrite(motorB2Pin, HIGH);
  } else {
    digitalWrite(motorA1Pin, LOW);
    digitalWrite(motorA2Pin, LOW);
    digitalWrite(motorB1Pin, LOW);
    digitalWrite(motorB2Pin, LOW);
  }
  delay(500);  // <= has to be "500" milliseconds
}

// Initialization function, run only once
void setup() {
  DebugBegin(9600);
  ppm.begin(receiverPin, false);
}

// Main function, run in a loop
void loop() {

  // get channels values
  short getSafeMode = ppm.read_channel(channelSafeMode);
  short getSpeedLevel = ppm.read_channel(channelSpeedLevel);
  short getLeftRight = ppm.read_channel(channelLeftRight);
  short getForwardReverse = ppm.read_channel(channelForwardReverse);

  // determinate limits, so the misconfigured receiver can not break code
  carOn = (getSafeMode < lowPositive) ? false : true;

  getLeftRight = (getLeftRight < lowNegative) ? lowNegative : getLeftRight;
  getLeftRight = (getLeftRight > highPositive) ? highPositive : getLeftRight;

  getForwardReverse = (getForwardReverse < lowNegative) ? lowNegative : getForwardReverse;
  getForwardReverse = (getForwardReverse > highPositive) ? highPositive : getForwardReverse;

  // parsing of the code, used to move the car in the desired direction
  if (carOn == true) {

    DebugPrint(F("Car is ON  |"));

    // get the throttle index, that is the persentage of throttle the driver behing the TX want
    if ((motorDirection < 0 && getForwardReverse >= highNegative) || (motorDirection > 0 && getForwardReverse <= lowPositive)) {
      // STOP => we are changing direction and would not want to overload the motor driver
      motorDirection = 0;
      throttleIndex = 0;
      stopBothMotors(false);
    } else if (getForwardReverse < lowPositive && getForwardReverse > highNegative) {
      // STOP => the TX stick is in neutral position
      motorDirection = 0;
      throttleIndex = 0;
    } else if (getForwardReverse >= lowPositive) {
      // FORWARD
      throttleIndex = map(getForwardReverse, lowPositive, highPositive, 0, 100);
      motorDirection = 1;
    } else if (getForwardReverse <= highNegative) {
      // REVERSE
      throttleIndex = map(getForwardReverse, highNegative, lowNegative, 0, 100);
      motorDirection = -1;
    }

    if (motorDirection >= 0) {
      DebugPrint(F("  "));
    } else {
      DebugPrint(F(" "));
    }

    DebugPrint(motorDirection);
    DebugPrint(F(" | "));
    DebugPrint(throttleIndex);
    DebugPrint(F(" | "));

    // get the turning index as initial index of power for each motor
    if (getLeftRight <= lowPositive && getLeftRight >= highNegative) {
      // CENTER
      motorAIndex = 100;
      motorBIndex = 100;
    } else if (getLeftRight < highNegative) {
      // LEFT
      //      if (motorDirection == -1) {
      //        // REVERSE
      //        motorAIndex = map(getLeftRight, lowNegative, highNegative, 0, 100);
      //        motorBIndex = 100;
      //      } else if (motorDirection == 1) {
      //        // FORWARD
      motorAIndex = 100;
      motorBIndex = map(getLeftRight, lowNegative, highNegative, 0, 100);
      //      }
    } else if (getLeftRight > lowPositive) {
      // RIGHT
      //      if (motorDirection == -1) {
      //        // REVERSE
      //        motorAIndex = 100;
      //        motorBIndex = map(getLeftRight, lowPositive, highPositive, 100, 0);
      //      } else if (motorDirection == 1) {
      //        // FORWARD
      motorAIndex = map(getLeftRight, lowPositive, highPositive, 100, 0);
      motorBIndex = 100;
      //      }
    }

    if (motorAIndex >= 0 && motorAIndex < 10) {
      DebugPrint(F("  "));
    } else {
      DebugPrint(F(""));
    }
    if (motorAIndex >= 10 && motorAIndex < 100) {
      DebugPrint(F(" "));
    } else {
      DebugPrint(F(""));
    }

    DebugPrint(motorAIndex);
    DebugPrint(F(" | "));

    if (motorBIndex >= 0 && motorBIndex < 10) {
      DebugPrint(F("  "));
    } else {
      DebugPrint(F(""));
    }
    if (motorBIndex >= 10 && motorBIndex < 100) {
      DebugPrint(F(" "));
    } else {
      DebugPrint(F(""));
    }

    DebugPrint(motorBIndex);
    DebugPrint(F(" | "));

    // calculate final power for each motor by multiply throttle and turn index for each motor
    motorAIndex = motorAIndex * throttleIndex;
    motorBIndex = motorBIndex * throttleIndex;

    //calculate final throttle level
    float finalThrottleLevel = 0;
    if (getSpeedLevel < highNegative) {
      finalThrottleLevel = 0.33;
    } else if (getSpeedLevel >= highNegative && getSpeedLevel <= lowPositive) {
      finalThrottleLevel = 0.66;
    } else {
      finalThrottleLevel = 1;
    }

    DebugPrint(" |");
    DebugPrint(255 * finalThrottleLevel);
    DebugPrint(" |");

    motorAIndex = map(motorAIndex, 0, 10000, 0, 255 * finalThrottleLevel);
    motorBIndex = map(motorBIndex, 0, 10000, 0, 255 * finalThrottleLevel);

    if (motorAIndex >= 0 && motorAIndex < 10) {
      DebugPrint(F("  "));
    } else {
      DebugPrint(F(""));
    }
    if (motorAIndex >= 10 && motorAIndex < 100) {
      DebugPrint(F(" "));
    } else {
      DebugPrint(F(""));
    }

    DebugPrint(motorAIndex);
    DebugPrint(F(" | "));

    if (motorBIndex >= 0 && motorBIndex < 10) {
      DebugPrint(F("  "));
    } else {
      DebugPrint(F(""));
    }
    if (motorBIndex >= 10 && motorBIndex < 100) {
      DebugPrint(F(" "));
    } else {
      DebugPrint(F(""));
    }

    DebugPrint(motorBIndex);
    DebugPrint(F(" | "));

    runMotorA(motorAIndex, motorDirection);
    runMotorB(motorBIndex, motorDirection);
  } else {
    motorDirection = 0;
    throttleIndex = 0;
    stopBothMotors(true);
    DebugPrint(F("Car is OFF |"));
  }

  DebugPrintln();
  delay(10);
}