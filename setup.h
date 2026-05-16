#ifndef ARMING_H
#define ARMING_H

#include <xc.h>

#include "mcc_generated_files/mcc.h"

// Time between main loop code execution
#define MAX_LOOP_TIME_DIFF_ms 500
// Time between "high speed" sensor checks
#define MAX_SENSOR_LOOP_TIME_DIFF_ms 5

// LEDs
#define WHITE_LED_ON() (LATC5 = 0)
#define WHITE_LED_OFF() (LATC5 = 1)
#define BLUE_LED_ON() (LATC6 = 0)
#define BLUE_LED_OFF() (LATC6 = 1)
#define RED_LED_ON() (LATC7 = 0)
#define RED_LED_OFF() (LATC7 = 1)



// Buzzer
#define BUZZER_ON() (LATB2 = 1)
#define BUZZER_OFF() (LATB2 = 0)

// Altimeters
#define ARM_A1() (LATB3 = 1)
#define DISARM_A1() (LATB3 = 0)

// Voltage Monitoring
#define ANALOG_SCALAR 3.7
// Current Monitorring
#define BATT_CURR_SCALAR 2.50 // mA/mV
#define CAN_CURR_SCALAR 0.05 // mA/mV

void output_init(void);

void osc_init(void);

#endif /* ARMING_H */

