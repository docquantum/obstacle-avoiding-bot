/**
 * Daniel Shchur
 * 2019-03-25
 * 
 * Main programming of an autonomous robot built for an
 * embedded systems course. Relies on separately implemented
 * Ultrasonic and Motor libraries that contain functions to
 * help with the algorithms used here. 
 * 
 * Currently, the robot follows a wall ~12 inches consistently.
 * Must be a flat straight wall. If the wall is not flat, the
 * movement is not consistent, but the robot moves towards/away
 * from the wall to compensate.
 * 
 * Relies on the ultrasonic sensor to know how far away it is
 * from the wall in question.
 * 
 * Future:
 * Will need to be able to navigate a "maze" environment without
 * getting stuck or running into wall.
 * 
*/ 

#include <Arduino.h>
#include "motors.h"
#include "ultrasonic.h"

#define TURN_DELAY 400

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
uint8_t state = 0;
uint8_t turnCount = 0;

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
 * Once it's in a corner, it turns 90 degrees;
 * moves forward until open space, turns -90 degrees;
 * repeats 2 times,
 * 
 * Once it's in the run toward the wall again, it turns 90 degrees
 * at the corner and resets to the first state of following and
 * adjusting.
*/ 
void followWall() {
  switch (state) {
  case 1:
    {
      //Get distances
      rotateSensor(0);
      delay(25);
      uint16_t frontDistance = getDistance();
      rotateSensor(wallSide);
      delay(25);
      uint16_t wallDistance = getDistance();
      // front is clear
      if(frontDistance > 13){
        moveForward();
        // wall too close
        if(wallDistance < 10){
          // adjust away from wall
          stopLeft(2.3*(24-wallDistance));
        } 
        // wall too far
        else if(wallDistance > 12){
          // adjust towards wall
          stopRight(2.3*wallDistance);
        }
      } 
      // front too close
      else {
        stopMotors();
        // turn 90
        turnLeft();
        delay(TURN_DELAY);
        stopMotors();
        delay(100);
        //increment turn count
        turnCount++;
        //switch to state 2
        state = 2;
        // rotate sensor towards wall
        rotateSensor(wallSide);
        delay(50);
      }
    }
    break;
  
  case 2:
    {
      if(turnCount > 2){
        stopMotors();
        state = 3;
        break;
      }
      uint16_t wallDistance = getDistance();
      moveForward();
      // turn 90 degrees
       if(wallDistance >= 24) {
        moveForward();
        // drive forward a bit
        delay(600);
        stopMotors();
        delay(100);
        // turn 90
        turnRight();
        delay(TURN_DELAY+10);
        stopMotors();
        delay(200);
        while(getDistance() > 24){
          moveForward();
          //stopMotors();
          delay(50);
        }
        stopMotors();
        //increment state 2 counter
        turnCount++;
      }
      // give time for US to be clear
      delay(50);
    }
    break;
  
  case 3:
    {
      //Get distances
      rotateSensor(0);
      delay(25);
      uint16_t frontDistance = getDistance();
      rotateSensor(wallSide);
      delay(25);
      uint16_t wallDistance = getDistance();
      // front is clear
      if(frontDistance > 13){
        moveForward();
        // wall too close
        if(wallDistance < 11){
          // adjust away from wall
          stopLeft(40);
        } 
        // wall too far
        else if(wallDistance > 13){
          // adjust towards wall
          stopRight(40);
        }
      } 
      // front too close
      else {
        stopMotors();
        // turn 90
        turnLeft();
        delay(TURN_DELAY);
        stopMotors();
        delay(100);
        //switch to state 1
        state = 1;
      }
    }
    break;
  
  default:
    {
      state = 1;
    }
    break;
  }
}

void setup() {
  setUpMotors(192, 185);
  setUpUltraSonic();
  testServo();
  wallSide = 2;
  rotateSensor(wallSide);
}

void loop() {
  followWall();
}
