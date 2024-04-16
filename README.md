# RC car
**Use (TGY) iA6C receiver and Arduino Nano board to save old and broken Nikko VaporizR 2 RC car.**

## Description

### Materials
- 1x Arduino Nano board
- 1x radio receiver (TGY-iA6C)
- 2x 260 mini DC motor (3V - 12V)
- 1x ZK-BM1 DC motor drive module (10A, 3V - 18V)
- 1x  AMS1117 step down power module (6.5V - 12V => 5V)
- wires

### Used libraries
- Library for PPM communication: [Arduino-PPM](https://github.com/Lynxmotion/Arduino-PPM)

**!! Thank you all for your work !!**

## Installation and Usage
1. Solder everything
2. Test connections
3. Clone the repository
4. Compile and upload the code to the Arduino
5. Test everything
6. Enjoy

## Custumizations

## Roadmap
### 2.0.0
*might never happen*
- add SD card reader for data logging
- add GPS for position and speed logging
- add camera module
- add sensors for autonomous drive

### 1.0.0
- code cleanup

### 0.4.0
- code optimization

### 0.3.0
- calculate motor power from receiver data

## Change Log
### 0.2.2
- combine throttle and turn indexes into unified motor power for each motor

### 0.2.1
- optimize data parsing
- detect critical points and act accordingly to prevent motor damage

### 0.2.0
- get data from the receiver
- parse data into human readable from

### 0.1.1
- change planned iBus protocol with PPM - more suitable for our receiver, since we
  can connect our receiver on any port of the Arduino (exept TX and RX ports) and
  not block sketch upload.

### 0.1.0
- initial setup
