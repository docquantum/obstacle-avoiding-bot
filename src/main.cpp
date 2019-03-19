#include <Arduino.h>
#include "motors.h"
#include "ultrasonic.h"

/* Pin Layout
 *
 * HBridge - Motors - Timer0
 * PWM_L = pin06 = PD6 = OC0A
 * IN1   = pin13 = PB5
 * IN2   = pin12 = PB4
 * IN3   = pin02 = PD4
 * IN4   = pin04 = PD2
 * PWM_R = pin05 = PD5 = OC0B
 * 
 * Ultrasonic - Timer1
 * TRIG = pin07 = PD7
 * ECHO = pin08 = PD0 = INP
 * 
 * Servo - Timer2
 * PWM = pin11 = PB3 = OC2A
 * POS = pinA0 = PC0
*/

int8_t wallSide = 0;

//returns -2 for left, 2 for right
int8_t findWall(){
  rotateSensor(2);
  delay(200);
  uint16_t rightDistance = getDistance();
  rotateSensor(-2);
  delay(200);
  uint16_t leftDistance = getDistance();
  rotateSensor(0);
  return (leftDistance < rightDistance) ? -2 : 2;
}

// Checks front, if no obstacles, drive forward.
//    else, move backward (for now)
// Checks distance to wall.
//    If distance changes, adjust accordingly.
void followWall(int8_t wallSide){
  rotateSensor(0);
  delay(30);
  if(getDistance() > 10){
    moveForward();
  } else{
    moveBackward();
  }
  rotateSensor(wallSide);
  delay(30);
  uint16_t curDist = getDistance();
  if(curDist < 10){
    if(wallSide < 0)
      stopRight(12 - (uint16_t)curDist*1.2 + 20);
    else
      stopLeft(12 - (uint16_t)curDist*1.2 + 20);
  } else if(curDist > 12){
    if(wallSide < 0)
      stopLeft((uint16_t)curDist*1.1 - 12 + 20);
    else
      stopRight((uint16_t)curDist*1.1 - 12 + 20);
  }
}

void setup() {
  setUpMotors(180, 180);
  setUpUltraSonic();
  testServo();
  wallSide = findWall();
}

void loop() {
  followWall(wallSide);
}
