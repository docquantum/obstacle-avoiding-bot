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

typedef enum {
  FORWARD,
  BACKWARD,
  TURN_RIGHT,
  TURN_LEFT
} SimpleState;

int8_t wallSide = 0;
State curState = START_STATE;
State prevState = START_STATE;
SimpleState state;



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
//    else, stop
// Checks distance to wall.
//    If distance changes, adjust accordingly.
void followWall(){
  rotateSensor(0);
  delay(30);
  uint16_t frontDistance = getDistance();
  if(frontDistance > 10){
    moveForward();
  } else {
    stop();
  }

  rotateSensor(wallSide);
  delay(30);
  uint16_t wallDistance = getDistance();
  //if front is close but wall is clear
  if(wallDistance < 10 && wallDistance >= 12 && wallDistance <= 10){
    //check other wall
    rotateSensor(-wallSide);
    delay(30);
    //if other wall is clear, turn ~90 degrees
    if(getDistance() > 12){
      if(wallSide == -2){
        turnRight();
        delay(500);
        stop();
      }
      //Else, turn around and reset wall side
    } else{
      turnRight();
      delay(700);
      stop();
      wallSide = findWall();
    }
  //if wall is really close, turn away hard from the wall
  } else if(wallDistance < 4){
    stop();
    if(wallSide == -2){
      turnRight();
      delay(250);
      stop();
      delay(50);
      moveForward();
      delay(300);
      stop();
      delay(50);
      turnLeft();
    } else {
      turnLeft();
      delay(250);
      stop();
      delay(50);
      moveForward();
      delay(300);
      stop();
      delay(50);
      turnRight();
    }
    delay(150);
    stop();
  //if wall is too close, turn away from wall a little
  } else if(wallDistance < 10){
    if(wallSide == -2){
      stopRight(20);
    } else{
      stopLeft(20);
    }
  } else if(wallDistance > 16){
    stop();
    if(wallSide == -2){
      turnLeft();
      delay(250);
      stop();
      delay(50);
      moveForward();
      delay(300);
      stop();
      delay(50);
      turnRight();
    } else {
      turnRight();
      delay(250);
      stop();
      delay(50);
      moveForward();
      delay(300);
      stop();
      delay(50);
      turnLeft();
    }
    delay(150);
    stop();
  } else if(wallDistance > 12){
    if(wallSide == -2){
      stopLeft(20);
    } else{
      stopRight(20);
    }
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
