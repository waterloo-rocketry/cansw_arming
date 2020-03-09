#include "altitude_parsing.h"

uint8_t rx_pool[32]; //32 bytes should be plenty

static srb_ctx_t rx_buf;

void uart1_rx_init(uint32_t baud, uint32_t osc_freq){
    
    // set up a ring buffer for receiving data
    srb_init(&rx_buf, rx_pool, sizeof(rx_pool), sizeof(uint8_t));
    
    U1CON0bits.BRGS = 0; //set up normal speed for baud rate generator
    U1CON0bits.ABDEN = 0; //disable Auto baud detect
    // not going to touch TXEN
    U1CON0bits.RXEN = 1; //enable receiver
    U1CON0bits.MODE = 0b0000; //Asynchronous 8-bit UART mode
    // set baud rate:
    uint16_t baud_rate_gen = (osc_freq / (16 * baud)) - 1;
    U1BRGH = baud_rate_gen >> 8 & 0xFF;
    U1BRGL = baud_rate_gen & 0xFF;
    
    U1RXPPS = RC4; //set C4 as RX
    ANSELC4 = 0;
    U1CON1bits.ON = 1;

    
    TRISC4 = 1; // set C4 as input
    
    PIE3bits.U1RXIE = 1; // enable interrupts which PIE do i want?
    
}

void uart1_handle_interupt(void){
    srb_push(&rx_buf, &U1RXB);
}
