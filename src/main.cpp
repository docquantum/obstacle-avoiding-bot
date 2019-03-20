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

typedef enum {
  START_STATE,
  WALL_CLEAR_FRONT_CLEAR,
  WALL_CLEAR_FRONT_CLOSE,
  WALL_CLOSE_FRONT_CLOSE,
  WALL_CLOSE_FRONT_CLEAR,
  WALL_FAR_FRONT_CLEAR,
  WALL_FAR_FRONT_CLOSE
} State;

int8_t wallSide = 0;
State curState = START_STATE;
State prevState = START_STATE;



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
void followWall(){
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
  if(curDist < 5){
    stop();
    if(wallSide == -2){
      turnRight();
    } else {
      turnLeft();
    }
    delay(100);
    stop();
  } else if(curDist < 10){
    if(wallSide < 0)
      stopRight(12 - (uint16_t)curDist*1.1 + 20);
    else
      stopLeft(12 - (uint16_t)curDist*1.1 + 20);
  } else if(curDist > 15){
    stop();
    if(wallSide == -2){
      turnLeft();
    } else {
      turnRight();
    }
    delay(100);
    stop();
  } else if(curDist > 12){
    if(wallSide < 0)
      stopLeft((uint16_t)curDist*1.1 - 12 + 20);
    else
      stopRight((uint16_t)curDist*1.1 - 12 + 20);
  }
}
/**
 * New implementation of the followWall function with distinct states
 * the robot will be in depending on previous states and surroundings.
 * 
 * Should help make it more robust as the previous function was too
 * "dumb" to deal with a few situations where the bot would turn too much. 
*/ 
void followWallFSM(){
  rotateSensor(0);
  uint16_t frontDistance = getDistance();
  rotateSensor(wallSide);
  uint16_t wallDistance = getDistance();
  Serial.print(frontDistance); Serial.print("\t"); Serial.print(wallDistance); Serial.print("\n\n");

  switch (curState) {
    case START_STATE:
      if(frontDistance > 10 && wallDistance > 10 && wallDistance < 12){
        curState = WALL_CLEAR_FRONT_CLEAR;
      } else if(frontDistance <= 10 && wallDistance > 10 && wallDistance < 12){
        curState = WALL_CLEAR_FRONT_CLOSE;
      } else if(frontDistance <= 10 && wallDistance <= 10){
        curState = WALL_CLOSE_FRONT_CLOSE;
      } else if(frontDistance > 10 && wallDistance <= 10){
        curState = WALL_CLOSE_FRONT_CLEAR;
      } else if(frontDistance > 10 && wallDistance >= 12){
        curState = WALL_FAR_FRONT_CLEAR;
      } else if(frontDistance <= 10 && wallDistance >= 12){
        curState = WALL_FAR_FRONT_CLOSE;
      }
      break;
    
    case WALL_CLEAR_FRONT_CLEAR:
      setSpeed(LEFT_SPEED, 200);
      setSpeed(RIGHT_SPEED, 200);
      moveForward();
      if(prevState == WALL_CLOSE_FRONT_CLOSE || prevState == WALL_CLOSE_FRONT_CLEAR){
        if(wallSide == -2){
          stopRight(20);
        } else{
          stopLeft(20);
        }
      } else if(prevState == WALL_FAR_FRONT_CLEAR || prevState == WALL_FAR_FRONT_CLOSE){
        if(wallSide == -2){
          stopLeft(20);
        } else{
          stopRight(20);
        }
      }
      // Go back to evaluation
      curState = START_STATE;
      prevState = WALL_CLEAR_FRONT_CLEAR;
      break;
    
    case WALL_CLEAR_FRONT_CLOSE:
      stop();
      rotateSensor(-wallSide);
      if(getDistance() > 12){
        if(wallSide == -2){
          turnRight();
          delay(500);
          stop();
        } else {
          turnLeft();
          delay(500);
          stop();
        }
      } else {
        moveBackward();
      }
      curState = START_STATE;
      prevState = WALL_CLEAR_FRONT_CLOSE;
      break;
    
    case WALL_CLOSE_FRONT_CLOSE:
      setSpeed(LEFT_SPEED, 120);
      setSpeed(RIGHT_SPEED, 120);
      moveBackward();
      if(prevState == WALL_CLEAR_FRONT_CLEAR){
        if(wallSide == -2){
          stopRight(30);
        } else {
          stopLeft(30);
        }
      } else if(prevState == WALL_CLOSE_FRONT_CLEAR){
        if(wallSide == -2){
          turnRight();
          delay(500);
          stop();
        } else{
          turnLeft();
          delay(500);
          stop();
        }
      }
      curState = START_STATE;
      prevState = WALL_CLOSE_FRONT_CLOSE;
      break;
    
    case WALL_CLOSE_FRONT_CLEAR:
      setSpeed(LEFT_SPEED, 200);
      setSpeed(RIGHT_SPEED, 200);
      moveForward();
      if(prevState == WALL_CLOSE_FRONT_CLOSE || prevState == WALL_CLOSE_FRONT_CLEAR){
        if(wallSide == -2){
          stopRight(30);
        } else {
          stopLeft(30);
        }
      } else{
        if(wallSide == -2){
          stopRight(20);
        } else {
          stopLeft(20);
        }
      }
      curState = START_STATE;
      prevState = WALL_CLOSE_FRONT_CLEAR;
      break;
    
    case WALL_FAR_FRONT_CLEAR:
      setSpeed(LEFT_SPEED, 120);
      setSpeed(RIGHT_SPEED, 120);
      moveForward();
      if(wallSide == -2){
        stopLeft(20);
      } else {
        stopRight(20);
      }
      curState = START_STATE;
      prevState = WALL_FAR_FRONT_CLEAR;
      break;

    case WALL_FAR_FRONT_CLOSE:
      setSpeed(LEFT_SPEED, 120);
      setSpeed(RIGHT_SPEED, 120);
      moveBackward();
      if(wallSide == -2){
        stopLeft(20);
      } else {
        stopRight(20);
      }
      curState = START_STATE;
      prevState = WALL_FAR_FRONT_CLOSE;
      break;
    
    default:
      // in case something goes wrong, reset to start state.
      curState = START_STATE;
      break;
  }
}

void setup() {
  setUpMotors(150, 150);
  setUpUltraSonic();
  testServo();
  wallSide = findWall();
}

void loop() {
  followWall();
}
