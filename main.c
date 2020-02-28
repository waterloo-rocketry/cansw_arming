#include <stdio.h>
#include <stdlib.h>
#include "arming.h"

#define _XTAL_FREQ 12000000 //12MHz

int main(int argc, char** argv) {
    
    output_init(); // initialize our outputs
    
    DISARM_A1(); //this is so that they don't always buzz during testing, REMOVE BEFORE FLIGHT
    DISARM_A2();
    
    while(1){
        
        __delay_ms(500);
        WHITE_LED_ON();
        __delay_ms(500);
        WHITE_LED_OFF();
        
    }
    return (EXIT_SUCCESS);
}

