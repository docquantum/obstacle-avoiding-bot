#define SAMPLE_SIZE       34
#define IR_DECODER_PIN    (PIND & (1 << PIND2))
#define IR_PIN            PD2
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

/**
 * Sets up the IR
*/ 
void setUpIR();

/*
 * Runs in main loop, decodes IR
 * and runs functions accordingly
 */
void decodeIR();