/**
 * Daniel Shchur
 * 2019-03-25
 * 
 * Motor library implementation that has standard functions to
 * manipulate robot movement via the two motors. Built specifically
 * for the robot wired in this project. Pin mappings would need to
 * change if transferred to another robot.
*/ 

#include <Arduino.h>
#include <avr/io.h>
#include <motors.h>

/*
 * HBridge - Motors - Timer0
 * PWM_L = pin06 = PD6 = OC0A
 * IN1   = pin13 = PB5
 * IN2   = pin12 = PB4
 * IN3   = pin02 = PD4
 * IN4   = pin04 = PD2
 * PWM_R = pin05 = PD5 = OC0B
*/

void setUpMotors(uint8_t left, uint8_t right){
    //Pins running PWM for h-bridge enable
    DDRD |= (1 << PD6) | (1 << PD5); //Set pins 6/5 as output
    PORTD |= (1 << PD6) | (1 << PD5); //Set pins 6/5 as HIGH

    // Sets timer0 to ~60hz and fastPWM mode
    TCCR0A |= 1 << COM0A1 | 1 << COM0B1 | 1 << WGM01 | 1 << WGM00;
    TCCR0B |=  0 << CS02 | 1 << CS01 | 1 << CS00;
    
    //Sets the duty cycle (Speed)
    LEFT_SPEED = left;
    RIGHT_SPEED = right;

    //Sets up the H-bridge IN pins
    DDRB |= (1 << PB5) | (1 << PB4); // Set pins 13/12 as output
    DDRD |= (1 << PD4) | (1 << PD2); // Set pins 4/2 as output
}

void setSpeed(uint8_t side, uint8_t speed){
    side = speed;
}

void moveForward(){
    PORTB = (PORTB & ~(1 << PB5)) | 1 << PB4; // Left
    PORTD = (PORTD & ~(1 << PD4)) | 1 << PD2; // Right
}

void moveBackward(){
    PORTB = (PORTB & ~(1 << PB4)) | 1 << PB5;
    PORTD = (PORTD & ~(1 << PD2)) | 1 << PD4;
}

void turnLeft(){
    PORTB = (PORTB & ~(1 << PB4)) | 1 << PB5;
    PORTD = (PORTD & ~(1 << PD4)) | 1 << PD2;
}

void turnRight(){
    PORTB = (PORTB & ~(1 << PB5)) | 1 << PB4;
    PORTD = (PORTD & ~(1 << PD2)) | 1 << PD4;
}

void stop(){
    PORTB &= ~(1 << PB5 | 1 << PB4);
    PORTD &= ~(1 << PD4 | 1 << PD2);
}

void stopLeft(uint8_t time){
    uint8_t oldPORTB = PORTB;
    PORTB &= ~(1 << PB5 | 1 << PB4);
    delay(time);
    PORTB = oldPORTB;
}

void stopRight(uint8_t time){
    uint8_t oldPORTD = PORTD;
    PORTD &= ~(1 << PD4 | 1 << PD2);
    delay(time);
    PORTD = oldPORTD;
}

void testMotors(){
    moveForward();
    delay(1000);
    stop();
    delay(500);

    moveBackward();
    delay(1000);
    stop();
    delay(500);

    turnLeft();
    delay(200);
    stop();
    delay(500);

    turnRight();
    delay(200);
    stop();
    delay(500);

    turnLeft();
    delay(500);
    stop();
    delay(500);

    turnRight();
    delay(500);
    stop();
    delay(500);
}

