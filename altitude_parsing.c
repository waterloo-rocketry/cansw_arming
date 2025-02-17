#include <xc.h>
#include <string.h>

#include "canlib/canlib.h"
#include "canlib/util/safe_ring_buffer.h"
#include "timer.h"

#include "altitude_parsing.h"

static int32_t altitude = -999;
static int32_t prev_altitude = -999;
static uint32_t time = 0;
static uint32_t prev_time = 0;
static int32_t velocity = 0;

static bool new_altitude = false;
static bool new_velocity = false;
static char string[16]; //string for new altitude data

char rx_pool[32]; // 32 bytes should be plenty

static srb_ctx_t rx_buf;

int32_t get_altitude(void){
    new_altitude = false;
    return altitude;
}

int32_t get_velocity(void){
    new_velocity = false;
    return velocity;
}

bool new_altitude_available(void){
    return new_altitude;
}

bool new_velocity_available(void){
    return new_velocity;
}

void uart1_rx_init(uint32_t baud, uint32_t osc_freq){

    memset(string, 0, strlen(string));
    // set up a ring buffer for receiving data
    srb_init(&rx_buf, rx_pool, sizeof(rx_pool), sizeof(char));

    U1CON0bits.RXEN = 1; // enable receiver
    U1CON0bits.BRGS = 0; // set up normal speed for baud rate generator
    U1CON0bits.ABDEN = 0; // disable Auto baud detect

    // not going to touch TXEN

    U1CON0bits.MODE = 0b0000; // Asynchronous 8-bit UART mode

    U1CON2bits.RUNOVF = 1; // keep running on overflow, never stop receiving
    U1CON2bits.FLO = 0; // disable flow control.
    // set baud rate:
    uint16_t baud_rate_gen = (osc_freq / (16 * baud)) - 1;
    U1BRGH = baud_rate_gen >> 8 & 0xFF;
    U1BRGL = baud_rate_gen & 0xFF;

    U1RXPPS = (0b010 << 3) | // port C
              (0b100);       // pin 4

    ANSELC4 = 0;
    TRISC4 = 1; // set C4 as input

    U1CON1bits.ON = 1; // enable UART

    PIE3bits.U1RXIE = 1; // enable interrupts
    U1ERRIEbits.FERIE = 1; // enable framing error interrupt
    U1ERRIEbits.RXFOIE = 1; // enable overflow error interrupt

}

void uart1_handle_interrupt(void){
    char rcv = U1RXB;
    srb_push(&rx_buf, &rcv);
}

void parse_altitude(void){
//    This function parses altitude data from a Stratologger CF altimeter
//    the data is just ASCII numbers followed by whitespace
//    For example:
//    1267
//    1328
//    1451
//    -6
//    Altitude data is received in ft

    static char element;

    while(!srb_is_empty(&rx_buf)){
        srb_pop(&rx_buf, &element);
        if((element & 0xF0) != 0 && strlen(string) < 15){    // if the second digit of the hex value is non zero we know it is not some form of whitespace
            strncat(string, &element, 1);
        }
        else if(strlen(string) > 0){ // if we hit a line ending, and our string has a number to read:
            altitude = strtol(string, NULL, 10);    // read the new altitude from the received string
            memset(string, 0, strlen(string));  //clear the string so we can start parsing again
            new_altitude = true;
            if(millis() - prev_time >= VELOCITY_ESTIMATION_INTERVAL){
                velocity = 1000*(altitude - prev_altitude)/(millis() - prev_time);
                prev_altitude = altitude; //store the previous altitude
                prev_time = millis(); //store the previous time
                new_velocity = true;
            }
            
        }
    }

}
