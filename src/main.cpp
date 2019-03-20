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

/**
 * After the wall side has been determined, follows that wall adjusting
 * it's direction depending on how far away it is from the wall.
 * 
*/ 
void followWall(){
  moveForward();
  delay(20);
  uint16_t wallDistance = getDistance();
  if(wallDistance <= 9){
    if(wallSide == -2){
      stopRight(20);
    } else{
      stopLeft(20);
    }
  } else if(wallDistance >= 13){
    if(wallSide == -2){
      stopLeft(20);
    } else{
      stopRight(20);
    }
  }
}

void setup() {
  setUpMotors(200, 200);
  setUpUltraSonic();
  testServo();
  wallSide = findWall();
  rotateSensor(wallSide);
  moveForward();
}

void loop() {
  followWall();
}
