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
    LATB4 = 0; // turn off the buzzer
    
    // Altimeter Arming
    TRISC3 = 0; // set C3 as an output for arming A1
    LATC3 = 1; // arm altimeter 1
    
    TRISC2 = 0; // set C2 as an output for arming A2
    LATC2 = 1; // arm altimeter 2
    
}

void osc_init(void){
    //Select external oscillator with PLL of 1:1
    OSCCON1 = 0b01110000;
    //wait until the clock switch has happened
    while (OSCCON3bits.ORDY == 0)  {}
    //if the currently active clock (CON2) isn't the selected clock (CON1)
    if (OSCCON2 != 0b01110000) {
        //infinite loop, something is broken, what even is an assert()?
        while (1) {}
    }
}

static uint32_t indicator_buzzer_last_millis = 0;
void indicator_buzzer_heartbeat(void){
    
    int loop_time = millis() - indicator_buzzer_last_millis;
    
    if((uint16_t)(ADCC_GetSingleConversion(BATTERY_1_PIN))*3.72 > UNDERVOLTAGE_THRESHOLD && loop_time < 750) 
        BUZZER_ON();
    
    else if(loop_time >= 750 && loop_time < 1000) BUZZER_OFF();
    
    else if((uint16_t)(ADCC_GetSingleConversion(BATTERY_2_PIN))*3.72 > UNDERVOLTAGE_THRESHOLD && loop_time >= 1000 && loop_time < 1250) 
        BUZZER_ON();
    
    else if(loop_time >= 1250 && loop_time < 1500) 
        BUZZER_OFF();
    
    else if((uint16_t)(ADCC_GetSingleConversion(BATTERY_2_PIN))*3.72 > UNDERVOLTAGE_THRESHOLD && loop_time >= 1500 && loop_time < 1750) 
        BUZZER_ON();
    
    else if(loop_time >= 1750 && loop_time < 2000) 
        BUZZER_OFF();

    else if(loop_time >= 2000) 
        indicator_buzzer_last_millis = millis();
    
}