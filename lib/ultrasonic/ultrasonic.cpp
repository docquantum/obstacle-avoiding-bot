/**
 * Daniel Shchur
 * 2019-03-25
 * 
 * Ultrasonic library implementation that has standard functions
 * to trigger, read, and rotate the ultrasonic sensor on robot.
 * Built specifically for the robot wired in this project.
 * Pin mappings would need to change if transferred to another
 * robot.
*/ 

#include <Arduino.h>
#include <avr/io.h>
#include <motors.h>

// Sets up vars that will be changed in interupts
volatile uint16_t count = 0;
volatile uint8_t distanceReady = 0;

void setUpServo(){
    DDRB |= (1 << PB3); // Set pin 3 as output
    PORTB |= (1 << PB3); //Set pin 3 as HIGH

    //Sets timer2 to ~60hz and fastPWM mode
    TCCR2A = 1 << COM2A1 | 1 << WGM21 | 1 << WGM20;
    TCCR2B =  1 << CS22 | 1 << CS21 | 1 << CS20;

    //Sets duty cycle to 0
    OCR2A = 0;
}

void setUpUltraSonic(){
    DDRD |= (1 << PD7); //Sets pin 7 for trigger output
    DDRB &= ~(0 << PB0); //Sets pin 8 for echo input

    //Init counter1
    TCCR1A = 0;
    //Set clock to count up clk/64 and falling edge
    TCCR1B |= 1 << CS11 | 1 << CS10;
    TCCR1C = 0;
    //Set counter to zero, high byte first
    TCNT1H = 0;
    TCNT1L = 0;
    // //Enable input capture interrupts
    // TIMSK1 |= (1 << ICIE1);
    // // Clear input capture and output compare A flag 
    TIFR1 |= (1 << OCF1A) | (1 << ICIE1);
    // Roll over every 40ms
    OCR1A = 10000;

    DDRD |= (1 << PD0); // Set pin 0 as output

    //Sets up the servo that turns the sensor
    setUpServo();
}

/**
 * Interupt Sub Routine that gets called when a falling edge event
 * is captured on the input capture pin for Timer1.
 * 
 * When falling edge triggers, get current timer value, then set
 * the "Distance Ready" flag to true. 
*/
// ISR (TIMER1_CAPT_vect){
//     //Get count
//     count = ICR1;
//     // Disable interrupt on match with OCR1A
//     TIMSK1 &= ~(1 << OCIE1A);
//     //Distance ready to be read
//     distanceReady = 1;
// }

void trigUltrasonic(){
    //distanceReady = 0;
    //Send pulse
    PORTD |= 1 << PD7;
    delayMicroseconds(10);
    PORTD &= ~(1 << PD7);
    // Clear Output Compare A flag 
    TIFR1 |= (1 << OCF1A);
    // Enable interrupt on match with OCR1A
    TIMSK1 |= (1 << OCIE1A);
}

/**
 * This function is called whenever the timer 1
 * output compare match OCR1A is generated.
*/
ISR(TIMER1_COMPA_vect){
    if(!(PINB & (1 << PINB0))){
        stopMotors();
        trigUltrasonic();
        if(PIND & (1 << PIND0)){
            PORTD &= ~(1 << PD0); //Toggle pin 0
        }else{
            PORTD |= (1 << PD0); //Toggle pin 0
        }
        
    }
}

uint16_t getDistance(){
    trigUltrasonic();
    // while echo is low
    while(!(PINB & (1 << PINB0)));
    //Clear timer
    TCNT1H = 0;
    TCNT1L = 0;
    // while echo is high
    while((PINB & (1 << PINB0)));
    // get count
    count = TCNT1;
    // Disable interrupt on match with OCR1A
    TIMSK1 &= ~(1 << OCIE1A);
    return (((count/(37))) < 0) ? 0 : ((count/(37)));
}

uint16_t readServoPos(){
    // Tell it to use internal Vcc
    ADMUX |= 1 << REFS0;
    // Start conversion
    ADCSRA |= 1 << ADSC;
    // Wait until conversion is done
    while(ADCSRA & (1 << ADSC));
    // Save value of registers (must be done in this order, else value gets discarded)
    uint8_t low  = ADCL;
    uint8_t high = ADCH;
    // OR the two register results together to get 16 bit val.
    return high << 8 | low;
}

/*
Servo Position Values
17  -2
162 -1
306  0
449  1
611  2
*/
void rotateSensor(int8_t position){
    if(position == 2) {
        OCR2A = 0; //90 Right
        while(readServoPos() > 22);
        delay(20);
    } else if(position == 1){
        OCR2A = 14; //45 Diagonal Right
        while(readServoPos() < 157 || readServoPos() > 167);
        delay(20);
    } else if(position == 0){
        OCR2A = 21; //0 Middle
        while(readServoPos() < 301 || readServoPos() > 311);
        delay(20);
    } else if(position == -1){
        OCR2A = 28; //-45 Diagonal Left
        while(readServoPos() < 444 || readServoPos() > 454);
        delay(20);
    } else if(position == -2){
        OCR2A = 36; //-90 Left
        while(readServoPos() < 606 || readServoPos() > 616);
        delay(20);
    }
}

void testServo(){
    rotateSensor(2);
    delay(350);
    rotateSensor(1);
    delay(350);
    rotateSensor(0);
    delay(350);
    rotateSensor(-1);
    delay(350);
    rotateSensor(-2);
    delay(350);
    rotateSensor(0);
    delay(350);
}