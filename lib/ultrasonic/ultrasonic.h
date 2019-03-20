
/**
 * Sets registers to set up the trigger pin and echo pin
 * as well as sets up Timer1 for input capture.
 * 
 * Calls internal function setUpServo which sets up the
 * servo the sensor is on by using Timer2 and setting up
 * the required PWM.
*/ 
void setUpUltraSonic();

/**
 * Resets the distanceReady flag and
 * sends a 10us pulse to the sensor.
*/ 
void trigUltrasonic();

/**
 * Triggers the sensor and waits for distance to be ready
 * before converting the count value to inches and
 * returning it.
 * 
 * (count * div) / (16 * 148) = inches
*/ 
uint16_t getDistance();

/**
 * Using a modified servo, reads the value off the pot-
 * entiometer on the analog pin A0 to give a reading of
 * where the servo arm is currently located.
 * 
 * Used to know when the servo is at the correct position
 * since it takes time for it to turn the sensor. 
*/ 
uint16_t readServoPos();

/**
 * Given a position (-2 to 2), rotates the sensor to that
 * position, waiting until the servo is at that position
 * using the readServoPos function, and then waiting 20ms
 * for good measure before returning back to the calling
 * function.
 * 
 * Values were determined via trial and error. 
 * 
 * Rotation is achieved by modifying the duty cycle of
 * Timer2.
*/ 
void rotateSensor(int8_t position);

/**
 * Rotates the sensor to the 5 positions ending in the middle
 * to show that it is working properly. 
*/ 
void testServo();