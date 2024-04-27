/* *************************************************************************** *
 * Project Title: RC car
 * 
 * Description: A small Arduino project that brings old and broken Nikko
 * VaporizR 2 car back to life. We use new motors, new motor driver board, 
 * an iA6C receiver module and a power step-down module to make use of the 
 * structure of the car useful again.
 *
 * Licence:  GPL 3.0       (details in project file "LICENCE")
 * Version:  0.4.1         (details in project file "README.md")
 * created:  14. 04. 2024
 * modified: 27. 04. 2024
 * by:       Luka Oman
 * *************************************************************************** */

// Declare additional libraries
#include <ppm.h>

// Define debuging messages
//#define DebugOn  // comment to not run debug

#ifndef DebugOn
#define DebugBegin(x)
#define DebugPrint(x)
#define DebugPrintln(x)
#else
#define DebugBegin(x) Serial.begin(x)
#define DebugPrint(x) Serial.print(x)
#define DebugPrintln(x) Serial.println(x)
#endif

// Define Arduino pins
static const int receiverPin = 3;
static const int motorA1Pin = 10;
static const int motorA2Pin = 11;
static const int motorB1Pin = 5;
static const int motorB2Pin = 6;

// Define new objects

// Define constants
#define channelSafeMode 5        // kill switch
#define channelSpeedLevel 6      // three position switch for controling maximum speed
#define channelLeftRight 1       // turn left or right
#define channelForwardReverse 2  // go forward or reverse
#define channelSteeringType 7    // three position switch for controling steering type

// in theory the signal can go from 1000 (low) to 1500 (neutral) and from 1500 (neutral) to 2000 (high)
// but in reallity those values can vary so we use user defined constants
static const int lowNegative = 1020;   // lowest signal received
static const int highNegative = 1480;  // low breaking point for neutral signal
static const int lowPositive = 1520;   // high breaking point for neutral signal
static const int highPositive = 1980;  // highest signal received
static const int middleMedium = 1500;  // middle signal received
static const long delayInterval = 50;  // nonblocking delay time in msec

// Define variables
unsigned long previousMillis = 0;  // timer for the nonblocking delay
int throttleIndex = 0;             // 0% => stop, 100% => full power
int motorDirection = 0;            // -1 => reverse, 0 => stop, 1 => forward
int motorAIndex = 0;               // left  motor index, 0% => stop, 100% full power
int motorBIndex = 0;               // right motor index, 0% => stop, 100% full power
short breakCounter = 0;            // count safe mode on to eliminate errors
bool motorABrake = false;          // true => motor A brakes, false => motor A runs
bool motorBBrake = false;          // true => motor B brakes, false => motor B runs
bool carOn = false;                // kill switch

// Define custom functions
void runMotorA(int throttle, int direction, bool brake) {

  if (brake == true) {
    digitalWrite(motorA1Pin, HIGH);
    digitalWrite(motorA2Pin, HIGH);
  } else {
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
}

void runMotorB(int throttle, int direction, bool brake) {

  if (brake == true) {
    digitalWrite(motorB1Pin, HIGH);
    digitalWrite(motorB2Pin, HIGH);
  } else {
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
}

void stopBothMotors(bool motorBreak) {
  // true if break (both HIGH), false is stop (both LOW)
  if (motorBreak == true) {
    DebugPrint(" BREAK");
    digitalWrite(motorA1Pin, HIGH);
    digitalWrite(motorA2Pin, HIGH);
    digitalWrite(motorB1Pin, HIGH);
    digitalWrite(motorB2Pin, HIGH);
  } else {
    DebugPrint("  STOP |");
    digitalWrite(motorA1Pin, LOW);
    digitalWrite(motorA2Pin, LOW);
    digitalWrite(motorB1Pin, LOW);
    digitalWrite(motorB2Pin, LOW);
  }
  delay(500);  // <= has to be "500" milliseconds
}

void driveCar() {

  // get channels values
  short getSafeMode = ppm.read_channel(channelSafeMode);
  short getSpeedLevel = ppm.read_channel(channelSpeedLevel);
  short getLeftRight = ppm.read_channel(channelLeftRight);
  short getForwardReverse = ppm.read_channel(channelForwardReverse);
  short getSteeringType = ppm.read_channel(channelSteeringType);

  // reset breaking status
  motorABrake = false;
  motorBBrake = false;

  // determinate limits, so the misconfigured receiver can not break code
  getSafeMode = (getSafeMode < lowNegative) ? lowNegative : getSafeMode;    // can not be lower than the channel minimum
  getSafeMode = (getSafeMode > highPositive) ? highPositive : getSafeMode;  // can not be higher than the channel maximum

  getSpeedLevel = (getSpeedLevel < lowNegative) ? lowNegative : getSpeedLevel;    // can not be lower than the channel minimum
  getSpeedLevel = (getSpeedLevel > highPositive) ? highPositive : getSpeedLevel;  // can not be higher than the channel maximum
  if (getSpeedLevel >= highNegative && getSpeedLevel <= lowPositive) {            // in the middle is in the middle
    getSpeedLevel = middleMedium;
  }

  getLeftRight = (getLeftRight < lowNegative) ? lowNegative : getLeftRight;    // can not be lower than the channel minimum
  getLeftRight = (getLeftRight > highPositive) ? highPositive : getLeftRight;  // can not be higher than the channel maximum
  if (getLeftRight >= highNegative && getLeftRight <= lowPositive) {           // in the middle is in the middle
    getLeftRight = middleMedium;
  }

  getForwardReverse = (getForwardReverse < lowNegative) ? lowNegative : getForwardReverse;    // can not be lower than the channel minimum
  getForwardReverse = (getForwardReverse > highPositive) ? highPositive : getForwardReverse;  // can not be higher than the channel maximum
  if (getForwardReverse >= highNegative && getForwardReverse <= lowPositive) {                // in the middle is in the middle
    getForwardReverse = middleMedium;
  }

  getSteeringType = (getSteeringType < lowNegative) ? lowNegative : getSteeringType;    // can not be lower than the channel minimum
  getSteeringType = (getSteeringType > highPositive) ? highPositive : getSteeringType;  // can not be higher than the channel maximum
  if (getSteeringType >= highNegative && getSteeringType <= lowPositive) {              // in the middle is in the middle
    getSteeringType = middleMedium;
  }

  carOn = (getSafeMode > lowNegative) ? true : false;

  // parsing of the code, used to move the car in the desired direction
  if (carOn == true) {

    DebugPrint(F("Car is ON  |"));

    breakCounter = 0;

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
      stopBothMotors(false);
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
    if (throttleIndex >= 0 && throttleIndex < 10) {
      DebugPrint(F("  "));
    } else if (throttleIndex >= 10 && throttleIndex < 100) {
      DebugPrint(F(" "));
    }
    DebugPrint(throttleIndex);

    switch (getSteeringType) {
      case lowNegative:       // switch in the minimum position
        getSteeringType = 0;  // speed of one wheel is decreasing with the sharper turn
        break;
      case middleMedium:      // switch in the middle position
        getSteeringType = 1;  // one wheel brakes
        break;
      case highPositive:      // switch in the maximum positon
        getSteeringType = 2;  // one wheel changes direction
        break;
    }

    DebugPrint(F(" | "));
    DebugPrint(getSteeringType);

    // get the turning index as initial index of power for each motor
    if (getLeftRight <= lowPositive && getLeftRight >= highNegative) {
      // CENTER
      motorAIndex = 100;
      motorBIndex = 100;
    } else if (getLeftRight < highNegative) {
      // LEFT
      if (getSteeringType == 0) {
        motorAIndex = map(getLeftRight, lowNegative, highNegative, 0, 100);
        motorABrake = false;
      } else if (getSteeringType == 1) {
        motorAIndex = 0;
        motorABrake = true;
      }
      motorBIndex = 100;
    } else if (getLeftRight > lowPositive) {
      // RIGHT
      motorAIndex = 100;
      if (getSteeringType == 0) {
        motorBIndex = map(getLeftRight, lowPositive, highPositive, 100, 0);
        motorBBrake = false;
      } else if (getSteeringType == 1) {
        motorBIndex = 0;
        motorBBrake = true;
      }
    }

    DebugPrint(F(" | "));
    if (motorAIndex >= 0 && motorAIndex < 10) {
      DebugPrint(F("  "));
    } else if (motorAIndex >= 10 && motorAIndex < 100) {
      DebugPrint(F(" "));
    }
    DebugPrint(motorAIndex);

    DebugPrint(F(" | "));
    DebugPrint(motorABrake);

    DebugPrint(F(" | "));
    if (motorBIndex >= 0 && motorBIndex < 10) {
      DebugPrint(F("  "));
    } else if (motorBIndex >= 10 && motorBIndex < 100) {
      DebugPrint(F(" "));
    }
    DebugPrint(motorBIndex);

    DebugPrint(F(" | "));
    DebugPrint(motorBBrake);

    // calculate final power for each motor by multiply throttle and turn index for each motor
    motorAIndex = motorAIndex * throttleIndex;
    motorBIndex = motorBIndex * throttleIndex;

    DebugPrint(F(" | "));
    if (motorAIndex >= 0 && motorAIndex < 10) {
      DebugPrint(F("    "));
    } else if (motorAIndex >= 10 && motorAIndex < 100) {
      DebugPrint(F("   "));
    } else if (motorAIndex >= 100 && motorAIndex < 1000) {
      DebugPrint(F("  "));
    } else if (motorAIndex >= 1000 && motorAIndex < 10000) {
      DebugPrint(F(" "));
    }
    DebugPrint(motorAIndex);

    DebugPrint(F(" | "));
    if (motorBIndex >= 0 && motorBIndex < 10) {
      DebugPrint(F("    "));
    } else if (motorBIndex >= 10 && motorBIndex < 100) {
      DebugPrint(F("   "));
    } else if (motorBIndex >= 100 && motorBIndex < 1000) {
      DebugPrint(F("  "));
    } else if (motorBIndex >= 1000 && motorBIndex < 10000) {
      DebugPrint(F(" "));
    }
    DebugPrint(motorBIndex);

    //calculate final throttle level
    float finalThrottleLevel = 0;
    if (getSpeedLevel < highNegative) {
      finalThrottleLevel = 0.33;
    } else if (getSpeedLevel >= highNegative && getSpeedLevel <= lowPositive) {
      finalThrottleLevel = 0.66;
    } else {
      finalThrottleLevel = 1;
    }

    DebugPrint(" | ");
    DebugPrint(finalThrottleLevel);

    finalThrottleLevel = 255 * finalThrottleLevel;
    DebugPrint(F(" | "));
    if (finalThrottleLevel >= 0 && finalThrottleLevel < 10) {
      DebugPrint(F("  "));
    } else if (finalThrottleLevel >= 10 && finalThrottleLevel < 100) {
      DebugPrint(F(" "));
    }
    DebugPrint(finalThrottleLevel);

    motorAIndex = map(motorAIndex, 0, 10000, 0, finalThrottleLevel);
    motorBIndex = map(motorBIndex, 0, 10000, 0, finalThrottleLevel);

    DebugPrint(F(" | "));
    if (motorAIndex >= 0 && motorAIndex < 10) {
      DebugPrint(F("  "));
    } else if (motorAIndex >= 10 && motorAIndex < 100) {
      DebugPrint(F(" "));
    }
    DebugPrint(motorAIndex);

    DebugPrint(F(" | "));

    if (motorBIndex >= 0 && motorBIndex < 10) {
      DebugPrint(F("  "));
    } else if (motorBIndex >= 10 && motorBIndex < 100) {
      DebugPrint(F(" "));
    }
    DebugPrint(motorBIndex);

    runMotorA(motorAIndex, motorDirection, motorABrake);
    runMotorB(motorBIndex, motorDirection, motorBBrake);
  } else {
    breakCounter++;
    if (breakCounter >= 5) {
      motorDirection = 0;
      throttleIndex = 0;
      DebugPrint(F("Car is OFF |"));
      stopBothMotors(true);
    }
  }

  DebugPrintln();
}

// Initialization function, run only once
void setup() {
  DebugBegin(9600);
  ppm.begin(receiverPin, false);
}

// Main function, run in a loop
void loop() {

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= delayInterval) {
    previousMillis = currentMillis;
    driveCar();
  }
}
