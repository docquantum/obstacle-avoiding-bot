#include <Arduino.h>
#include <avr/io.h>

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
    TCCR1A = 0; //Standard mode (Fall over at MAX)
    TCCR1B |= 1 << CS11 | 1 << CS10 | 1 << ICES1; //Set clock to count up clk/64 and rising edge
    TCCR1C = 0;
    //Set counter to zero, high byte first
    TCNT1H = 0;
    TCNT1L = 0;
    //Enable input capture interrupts
    TIMSK1 |= 1 << ICIE1;
    //Clears input capture flag
    TIFR1 = 0;

    //Sets up the servo that turns the sensor
    setUpServo();
}

/**
 * Interupt Sub Routine that gets called when an event is captured
 * on the input capture pin for Timer1
 * 
 * When rising edge triggers, reset timer and set to falling edge
 * detection.
 * 
 * When falling edge triggers, get current timer value, then set
 * back to rising edge detection for next reading, setting the
 * "Distance Ready" flag to true. 
*/
ISR (TIMER1_CAPT_vect){
    //On rising edge
    if (TCCR1B & (1<<ICES1)) {
        //Set to detect falling edge
        TCCR1B &= ~(1<<ICES1); 
        //Clear timer
        TCNT1H = 0;
        TCNT1L = 0;
    } else { //On falling edge
        //Reset detect falling edge
        TCCR1B |= (1<<ICES1); 
        //Get count
        count = ICR1;
        //Distance ready to be read
        distanceReady = 1; 
    }
}

void trigUltrasonic(){
    distanceReady = 0;
    PORTD |= 1 << PD7;
    delayMicroseconds(10);
    PORTD &= ~(1 << PD7);
}

uint16_t getDistance(){
    trigUltrasonic();
    while(!distanceReady){};
    return (((count/(37))) < 0) ? 0 : ((count/(37)));
}

uint16_t readServoPos(){
    // Tell it to use internal Vcc
    ADMUX |= 1 << REFS0;

    // Start conversion
    ADCSRA |= 1 << ADSC;

    // Wait until conversion is done
    while(ADCSRA & (1 << ADSC)){};

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
        while(readServoPos() > 22){};
        delay(10);
    } else if(position == 1){
        OCR2A = 14; //45 Diagonal Right
        while(readServoPos() < 156 || readServoPos() > 166){};
        delay(10);
    } else if(position == 0){
        OCR2A = 21; //0 Middle
        while(readServoPos() < 301 || readServoPos() > 411){};
        delay(10);
    } else if(position == -1){
        OCR2A = 28; //-45 Diagonal Left
        while(readServoPos() < 444 || readServoPos() > 464){};
        delay(10);
    } else if(position == -2){
        OCR2A = 36; //-90 Left
        while(readServoPos() < 606 || readServoPos() > 616){};
        delay(10);
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