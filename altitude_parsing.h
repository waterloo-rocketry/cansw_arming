#ifndef ALTITUDE_PARSING_H
#define	ALTITUDE_PARSING_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <xc.h>
#include <stdint.h>
#include <string.h>
#include "canlib/util/safe_ring_buffer.h"
    
    int32_t altitude;
    char string[16];

    void uart1_rx_init(uint32_t baud, uint32_t osc_freq);
    
    void uart1_handle_interupt(void);
    
    void parse_altitude(void);


#ifdef	__cplusplus
}
#endif

#endif	/* ALTITUDE_PARSING_H */

