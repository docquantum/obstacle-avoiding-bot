/**
 * Daniel Shchur
 * 2019-03-25
 * 
 * Motor library header that has standard functions to manipulate
 * robot movement via the two motors. Built specifically for the
 * robot wired in this project. Pin mappings would need to change
 * if transferred to another robot.
*/ 

#define LEFT_SPEED OCR0A
#define RIGHT_SPEED OCR0B

/**
 * Sets up the pins for the motors using register calls.
 * Also starts the PWM for the enable pins.
*/ 
void setUpMotors(uint8_t left, uint8_t right);

/**
 * Sets the speed of the given side (LEFT/RIGHT_SPEED);
 * speed: (0 to 255)
*/
void setSpeed(uint8_t side, uint8_t speed);

/**
 * Sets movement to forward;
 * Sets IN2 and IN3 to HIGH
*/ 
void moveForward();

/**
 * Sets movement to backward;
 * Sets IN1 and IN4 to HIGH
*/
void moveBackward();

/**
 * Turns left in place;
 * Sets IN1 and IN3 to HIGH
*/
void turnLeft();

/**
 * Turns right in place;
 * Sets IN2 and IN4 to HIGH
*/
void turnRight();

/**
 * Stops movement;
 * Sets all INs to LOW
*/
void stopMotors();

/**
 * Stops right Wheel for a given amount of time in ms;
 * Brings it back to previous state at end.
*/
void stopRight(uint16_t time);

/**
 * Stops left Wheel for a given amount of time in ms;
 * Brings it back to previous state at end.
*/
void stopLeft(uint16_t time);

/**
 * Cycles through the functions that
 * determine movement and direction.
*/ 
void testMotors();