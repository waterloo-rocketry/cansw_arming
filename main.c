#include <stdio.h>
#include <stdlib.h>

#include "canlib\can.h"
#include "canlib\can_common.h"
#include "canlib\pic18f26k83\pic18f26k83_can.h"
#include "canlib\message_types.h"
#include "canlib\util\timing_util.h"
#include "canlib\util\can_tx_buffer.h"
#include "canlib\pic18f26k83\pic18f26k83_timer.h"

#include "arming.h"
#include "altitude_parsing.h"
#include "mcc_generated_files\mcc.h"

#define _XTAL_FREQ 12000000 //12MHz

#define TOGGLE_TIME 500

static void can_msg_handler(can_msg_t *msg);

// Memory pool for CAN transmit buffer
uint8_t tx_pool[100];

int main(int argc, char** argv) {
    //init functions
    osc_init();
    timer0_init();    
    output_init(); // initialize our outputs
    uart1_rx_init(9600, _XTAL_FREQ); //initialize the UART module
    // MCC generated functions
    ADCC_Initialize(); 
    FVR_Initialize();
    
    // enable global interupts
    INTCON0bits.GIE = 1;
    
    /***********set up CAN**********/
    
    // Set up CAN TX
    TRISC0 = 0;
    RC0PPS = 0x33;

    // Set up CAN RX
    TRISC1 = 1;
    ANSELC1 = 0;
    CANRXPPS = 0x11;
    
    // set up CAN module
    can_timing_t can_setup;
    can_generate_timing_params(_XTAL_FREQ, &can_setup);

    can_init(&can_setup, can_msg_handler);
    // set up CAN tx buffer
    txb_init(tx_pool, sizeof(tx_pool), can_send, can_send_rdy);
    
    //DISARM_A1(); //this is so that they don't always buzz during testing, REMOVE BEFORE FLIGHT
    //DISARM_A2();
    
    unsigned long next_switch_time = 0;
    uint8_t on = 0;
    
    while(1){
        
        if(millis() > next_switch_time){
            next_switch_time = millis() + TOGGLE_TIME;
            if(on){
                //DISARM_A1();
                //ARM_A2();
                WHITE_LED_OFF();
                //BLUE_LED_ON();
                on = 0;
            }
            else{
                WHITE_LED_ON();
                //BLUE_LED_OFF();
                //ARM_A1();
                //DISARM_A2();
                on = 1;
            }
        }
        ADCC_GetSingleConversion(BATTERY_1_PIN)*3.72;
        
        parse_altitude();
        
        // send queued messages
        txb_heartbeat();
    }
    return (EXIT_SUCCESS);
}

static void __interrupt() interrupt_handler(){
    if (PIE3bits.TMR0IE == 1 && PIR3bits.TMR0IF == 1) {
        timer0_handle_interrupt();
        PIR3bits.TMR0IF = 0;
    }
    if(U1ERRIRbits.FERIF == 1 || U1ERRIRbits.RXFOIF){   //should probably do something if there is an error
        RED_LED_ON();
    }
    else if (PIE3bits.U1RXIE == 1 && PIR3bits.U1RXIF == 1){
        uart1_handle_interupt();
        PIR3bits.U1RXIF = 0;
    }
    else if(PIR3bits.U1EIF == 1){   //should probably do something if there is an error
        PIR3bits.U1EIF = 0;
    }
    
}

static void can_msg_handler(can_msg_t *msg) {
    uint16_t msg_type = get_message_type(msg);
    switch (msg_type) {
        
        default:
            // send a message or something
            break;
    }
}