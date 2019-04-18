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
#include "ir.h"

// Set built in reset function to PC 0
void(* resetFunc) (void) = 0;

volatile uint8_t newIrPacket = FALSE;
volatile uint32_t irPacket;
uint32_t oldIrPacket;
volatile uint8_t packetIndex;
volatile uint8_t dataReady;

/*
 * main loop
 */
void decodeIR() {
  // Waiting for new IR packet
  while(!newIrPacket);
  newIrPacket = FALSE;

  if(dataReady){
    // Atomicly get data, reset flags, parse info
    cli();
    oldIrPacket = irPacket;
    irPacket = 0x0;
    dataReady = 0;
    if(oldIrPacket == VOLUP_BTN){
      moveForward();
    } else if(oldIrPacket == VOLDOWN_BTN){
      moveBackward();
    } else if(oldIrPacket == LSKP_BTN){
      turnLeft();
    } else if(oldIrPacket == RSKP_BTN){
      turnRight();
    } else if(oldIrPacket == UP_BTN){
      LEFT_SPEED += 20;
      RIGHT_SPEED +=20;
    } else if(oldIrPacket == DOWN_BTN){
      LEFT_SPEED -= 20;
      RIGHT_SPEED -=20;
    } else if(oldIrPacket == PAUSE_BTN){
      stopMotors();
    } else if(oldIrPacket == POWER_BTN){
      stopMotors();
      resetFunc();
    } else if(oldIrPacket == FUNC_BTN){
      //set up logic to switch to auto move mode
    }
    //reset timer to make sure funky things stop from
    //happening when interupts are re-enabled.
    TCNT1H = 0;
    TCNT1L = 0;
    sei();
  }
}

/**
 * Sets up the IR
*/ 
void setUpIR() {
  // Turn off WDT 
  WDTCSR = 0x00; 

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

  // Set Pin 1 for IR input
	DDRD &= ~(1 << IR_PIN); // Set as input
	PORTD |= (1 << IR_PIN); // Pullup high
  
  // Enable External Interrupt Mask 0
  EIMSK |= (1 << INT0);

  // Set falling edge detection for pin 2 interupts
  SET_FALLING;
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
    uint16_t pulseWidth = TCNT1;
    if(packetIndex > 0 && packetIndex < 33){
      if(pulseWidth > 350) {
        // Leftshift and OR in 1 on MSB
        irPacket = (irPacket << 1) | 1;
      } else{
        // Leftshift
        irPacket = irPacket << 1;
      }
    } else if(packetIndex == 33){
      dataReady = 1;
    }
    packetIndex++;
    // Get rid of residual data on the first bit
    if(pulseWidth > 1200){
      packetIndex--;
    }
    // Disable interrupt on match with OCR1A
    TIMSK1 &= ~(1 << OCIE1A);
    // Enable rising edge detection
    SET_RISING;
  } else{
    // Clear counter
    TCNT1H = 0;
    TCNT1L = 0;
    // Clear Output Compare A flag 
    TIFR1 |= (1 << OCF1A);
    // Enable interrupt on match with OCR1A
    TIMSK1 |= (1 << OCIE1A);
    // Enable falling edge detection
    SET_FALLING;
  }
}