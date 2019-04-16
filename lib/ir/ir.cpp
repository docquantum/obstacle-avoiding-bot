/**
 * Name: Daniel Shchur
 * Date: 2019-04-08
 * 
 * Sets up IR reciever and decodes it, converting the codes
 * to inputs to drive the robot.
 * 
 * Reference:
 * Power: Reset     0xFFA25D
 * FUNC:  Auto      0xFFE21D
 * Vol+:  Forward   0xFF629D
 * Vol-:  Backward  0xFFA857
 * LSkp:  Left      0xFF22DD
 * RSkp:  Right     0xFFC23D
 * |>||:  Stop      0xFF02FD
 * Down:  Slower    0xFFE01F
 * Up:    Faster    0xFF906F
 * 
 * ELEGOO Remote:
 * Start  HIGH  ~2275
 * Start  LOW   ~1130
 * Stop   HIGH  ~13143
 * Stop   LOW   ~146
 * 0  HIGH  ~146
 * 0  LOW   ~146
 * 1  HIGH  ~146
 * 1  LOW   ~416  
*/

#include <Arduino.h>
#include <avr/io.h>
#include "motors.h"

#define SAMPLE_SIZE       34
#define IR_DECODER_PIN    (PIND & (1 << PIND2)) // (i.e. Digital Pin 2)
#define IR_PIN            (PIND2)
#define SET_FALLING       (EICRA = (EICRA & ~(1 << ISC00)) | (1 << ISC01))
#define SET_RISING        (EICRA |= (1 << ISC00) | (1 << ISC01))
// falling if 0, rising if 1
#define EDGE              (EICRA & (1 << ISC00))
#define TRUE              1
#define FALSE             0

// IR Data Constants
#define POWER_BTN   0xFFA25D
#define FUNC_BTN    0xFFE21D
#define VOLUP_BTN   0xFF629D
#define VOLDOWN_BTN 0xFFA857
#define LSKP_BTN    0xFF22DD
#define RSKP_BTN    0xFFC23D
#define PAUSE_BTN   0xFF02FD
#define DOWN_BTN    0xFFE01F
#define UP_BTN      0xFF906F

// Set built in reset function to PC 0
void(* resetFunc) (void) = 0;

volatile uint8_t newIrPacket = FALSE;
volatile uint32_t irPacket;
uint32_t oldIrPacket;
volatile uint8_t packetData[SAMPLE_SIZE];
volatile uint8_t packetIndex;
volatile uint8_t dataReady;

/*
 * main loop
 */
void decodeIR() {
  // Waiting for new IR packet
  while(!newIrPacket);

  if(dataReady){
    // Atomicly get data and reset data flag
    cli();
    oldIrPacket = irPacket;
    dataReady = 0;
    sei();
    if(oldIrPacket == VOLUP_BTN){
      moveForward();
    } else if(oldIrPacket == VOLDOWN_BTN){
      moveBackward();
    } else if(oldIrPacket == LSKP_BTN){
      turnLeft();
    } else if(oldIrPacket == RSKP_BTN){
      turnRight();
    } else if(oldIrPacket == UP_BTN){
      setSpeed(LEFT_SPEED,(LEFT_SPEED+5));
      setSpeed(RIGHT_SPEED,(RIGHT_SPEED+5));
    } else if(oldIrPacket == DOWN_BTN){
      setSpeed(LEFT_SPEED,(LEFT_SPEED-5));
      setSpeed(RIGHT_SPEED,(RIGHT_SPEED-5));
    } else if(oldIrPacket == PAUSE_BTN){
      stopMotors();
    } else if(oldIrPacket == POWER_BTN){
      resetFunc();
    } else if(oldIrPacket == FUNC_BTN){
      //set up logic to switch to auto move mode
    }
  }
}


/**
 * Sets up the IR
*/ 
void setUpIR() {
  // Turn off WDT 
  WDTCSR = 0x00; 
  
  Serial.begin(9600);
  Serial.println("Starting up.");

  // Reset settings
  TCCR1A = 0x0;
  TCCR1B = 0x0;
  // Set Timer 1 to CTC with OCR1A as top
  TCCR1A |= (1 << WGM12);
  // Set clock prescaler to clk/64
  TCCR1B |= 1 << CS11 | 1 << CS10;
  // Configure OCR1A to generate an interrupt every 70ms
  // where (sec * 16million)/div = count
  OCR1A = 17500; 
  // Clear Timer 1
  TCNT1H = 0;
  TCNT1L = 0;

  // Set Pin 2 for IR input
	DDRD &= ~(1 << PD2);  // Set as input
	PORTD |= (1 << PD2);	// Pullup high

}

/**
 * This function is called whenever the timer 1
 * output compare match OCR1A is generated.
*/
ISR(TIMER1_COMPA_vect){
  // Disable interrupt on match with OCR1A
  TIMSK1 &= ~(1 << OCIE1A);
  // Clear the packet index
  packetIndex = 0;
  // Ready for a new packet.
  newIrPacket = TRUE;
}


/**
 * Called when an edge change is detected
 * on the IR reciever pin. 
*/ 
ISR(INT0_vect){
  if(!EDGE){
    // Store count into packet data
    packetData[packetIndex] = TCNT1;
    if(packetIndex > 0 && packetIndex < 33){
      if(packetData[packetIndex] > 350) {
        // Leftshift and OR in 1 on MSB
        irPacket = (irPacket << 1) | 1;
      } else{
        // Leftshift
        irPacket = (irPacket << 1);
      }
    } else if(packetIndex == 33){
      dataReady = 1;
    }
    // Disable interrupt on match with OCR1A
    TIMSK1 &= ~(1 << OCIE1A);
    // Enable rising edge detection
    SET_RISING;
  } else{
    // Clear counter
    TCNT1 = 0;
    // Clear Output Compare A flag 
    TIFR1 |= (1 << OCF1A);
    // Enable interrupt on match with OCR1A
    TIMSK1 |= (1 << OCIE1A);
    // Enable falling edge detection
    SET_FALLING;
  }
}