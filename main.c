#include <stdio.h>
#include <stdlib.h>
#include "arming.h"
#include "altitude_parsing.h"
#include "canlib\pic18f26k83\pic18f26k83_timer.h"
#include "mcc_generated_files/mcc.h"

#define _XTAL_FREQ 12000000 //12MHz

#define TOGGLE_TIME 500

    uint16_t battery_1_voltage = 0;

int main(int argc, char** argv) {
    
    timer0_init();    
    output_init(); // initialize our outputs
    
    uart_rx_init(9600); //initialize the UART module
    
    ADCC_Initialize(); // MCC generated functions
    FVR_Initialize();
    
    INTCON0bits.GIE = 1; // enable global interupts
    
    DISARM_A1(); //this is so that they don't always buzz during testing, REMOVE BEFORE FLIGHT
    DISARM_A2();
    
    unsigned long next_switch_time = 0;
    uint8_t on = 0;

    
    
    while(1){
        
        if(millis() > next_switch_time){
            next_switch_time = millis() + TOGGLE_TIME;
            if(on){
                //DISARM_A1();
                //ARM_A2();
                WHITE_LED_OFF();
                BLUE_LED_ON();
                on = 0;
            }
            else{
                WHITE_LED_ON();
                BLUE_LED_OFF();
                //ARM_A1();
                //DISARM_A2();
                on = 1;
            }
        }
        battery_1_voltage = ADCC_GetSingleConversion(BATTERY_1_PIN)*3.72;
        
        
    }
    return (EXIT_SUCCESS);
}

static void __interrupt() interrupt_handler(){
    if (PIE3bits.TMR0IE == 1 && PIR3bits.TMR0IF == 1) {
        timer0_handle_interrupt();
        PIR3bits.TMR0IF = 0;
    }
}