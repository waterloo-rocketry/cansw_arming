#include "altitude_parsing.h"

void uart_rx_init(int baud){
    U1CON0bits.BRGS = 0; //set up normal speed for baud rate generator
    U1CON0bits.ABDEN = 0; //disable Auto baud detect
    // not going to touch TXEN
    U1CON0bits.RXEN = 1; //enable reciver
    U1CON0bits.MODE = 0b0000; //Asynchronous 8-bit UART mode
    U1BRGH = baud rate
    U1BRGL = baud rate
    RXPPS = RC4; // I think this is right?
    ANSELC4 = 0;
    U1CON1bits.ON = 1;
    
    TRISC4 = 1; // set C4 as input
    
    PIE1bits.U1RXIE = 1; // enable interupts which PIE do i want?
    
}
