#ifndef ALTITUDE_PARSING_H
#define ALTITUDE_PARSING_H

#include <stdbool.h>
#include <stdint.h>

#define VELOCITY_ESTIMATION_INTERVAL 100 // in ms

int32_t get_altitude(void);

int32_t get_velocity(void);

bool new_altitude_available(void);

bool new_velocity_available(void);

void uart1_rx_init(uint32_t baud, uint32_t osc_freq);

void uart1_handle_interrupt(void);

void parse_altitude(void);

#endif /* ALTITUDE_PARSING_H */

