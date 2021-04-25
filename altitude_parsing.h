#ifndef ALTITUDE_PARSING_H
#define	ALTITUDE_PARSING_H

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "canlib/util/safe_ring_buffer.h"

int32_t get_altitude(void);

bool new_altitude_available(void);

#include <xc.h>
#include "canlib/pic18f26k83/pic18f26k83_timer.h"

void uart1_rx_init(uint32_t baud, uint32_t osc_freq);

void uart1_handle_interrupt(void);

void parse_altitude(void);

#endif	/* ALTITUDE_PARSING_H */

