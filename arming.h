#ifndef ARMING_H
#define	ARMING_H

#ifdef	__cplusplus
extern "C" {
#endif
 
#include <xc.h>

// LEDs
#define RED_LED_ON() (LATC7 = 0)
#define RED_LED_OFF() (LATC7 = 1)
#define BLUE_LED_ON() (LATC6 = 0)
#define BLUE_LED_OFF() (LATC6 = 1)
#define WHITE_LED_ON() (LATC5 = 0)
#define WHITE_LED_OFF() (LATC5 = 1)
    
// Buzzer
#define BUZZER_ON() (LATB4 = 0)
#define BUZZER_OFF() (LATB4 = 1)

// Altimeters
#define ARM_A1() (LATC3 = 1)
#define DISARM_A1() (LATC3 = 0)
#define ARM_A2() (LATC2 = 1)
#define DISARM_A2() (LATC2 = 0)


  void output_init(void);

#ifdef	__cplusplus
}
#endif

#endif	/* ARMING_H */

