#ifndef ARMING_H
#define	ARMING_H
 
#include <xc.h>

// Time between main loop code execution
#define MAX_LOOP_TIME_DIFF_ms 1000
    
// LEDs
#define RED_LED_ON() (LATC7 = 0)
#define RED_LED_OFF() (LATC7 = 1)
#define BLUE_LED_ON() (LATC6 = 0)
#define BLUE_LED_OFF() (LATC6 = 1)
#define WHITE_LED_ON() (LATC5 = 0)
#define WHITE_LED_OFF() (LATC5 = 1)
    
// Buzzer
#define BUZZER_ON() (LATB4 = 1)
#define BUZZER_OFF() (LATB4 = 0)

// Altimeters
#define ARM_A1() (LATC3 = 1)
#define DISARM_A1() (LATC3 = 0)
#define ARM_A2() (LATC2 = 1)
#define DISARM_A2() (LATC2 = 0)
    
// Voltage Monitoring
#define BATTERY_1_PIN   channel_ANA0
#define BATTERY_2_PIN    channel_ANA1
#define A1_MAIN_PIN     channel_ANB3
#define A1_DROGUE_PIN   channel_ANB2
#define A2_MAIN_PIN     channel_ANB1
#define A2_DROGUE_PIN   channel_ANB0

#define ANALOG_SCALLER 3.72


  void output_init(void);
  
  void osc_init(void);

#endif	/* ARMING_H */

