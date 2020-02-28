#include "arming.h"


void output_init(void){
    // LEDs
    TRISC5 = 0; // set C5 as an output for the white LED
    LATC5 = 1; // turn the white LED off
    
    TRISC6 = 0; // set C6 as an output for the blue LED
    LATC6 = 1; // turn the blue LED off
    
    TRISC7 = 0; // set C7 as an output for the red LED
    LATC7 = 1; // turn the red LED off
    
    // Indicator Buzzer
    TRISB4 = 0; // set B4 as an output for the indicator buzzer
    LATB4 = 1; // turn off the buzzer
    
    // Altimeter Arming
    TRISC3 = 0; // set C3 as an output for arming A1
    LATC3 = 1; // arm altimeter 1
    
    TRISC2 = 0; // set C2 as an output for arming A2
    LATC2 = 1; // arm altimeter 2
    
}
