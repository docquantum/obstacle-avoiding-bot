# Intelligent Robot

This was an assignment for an embedded systems class where we use an Arduino Uno, a robot chassis, and a few sensors with motors to create a self driving, obstacle avoiding, and overall, intelligent robot.

## Wiring

There are two folders with relevant files on the repository. One has Fritzing schematic files, the other exports of the Fritzing schematics and photos of the robot itself.

TODO:
- Will be using Fritzing to create a wiring diagram
- Add photo of currently wired robot as well
- Add a table with pin out as well to see what pins on the arduino are connected to what. Also add the PORT names.

## Architecture

The project is built on the PlatformIO IDE framework which contains all the tools necessary to create proper code for embedded systems, such as the Arduino Uno. Although it is built on this system, the code can be easily adapted to the Arduino IDE which uses .ino files.

The project code is split into 3 directories for organization.
- 'src' will contain the main code and any non library code files.
- 'lib' contains custom libraries used in the project to help modularize and keep the code clean.
- 'include' contains any header files used by code in 'src' that's not a library.

### Libraries

There are three libraries in the project, all of which are used to interface with various peripherals of the robot.

#### Motors

This library contains functions and constants related to the driving of the motors on the robot

#### Servo

This library contains functions and constants related to the driving of the servo which has an ultrasonic sensor attached to it.

#### Ultrasonic

This library contains functions and constants related to the operation of the the ultrasonic sensor attached to the servo.

#### TODO
- Add more specific info about the libraries, such as common functions.

### Code

The code in this project relies minimally on Arduino standard functions. Instead, the code uses register manipulations to set various settings on the Atmel processor. The only functions that are used are ones typically for serial communications and delays.
