#ifndef ARMING_H
#define	ARMING_H

#include <xc.h>
#include "canlib/pic18f26k83/pic18f26k83_timer.h"
#include "mcc_generated_files/mcc.h"

#define FIELD_ASL 1000
// Time between main loop code execution
#define MAX_LOOP_TIME_DIFF_ms 1000
// Time between "high speed" sensor checks
#define MAX_SENSOR_LOOP_TIME_DIFF_ms 5
// Time after after which if we don't have an altitude we error.
#define MAX_ALTITUDE_INTERVAL_ms 2000

typedef enum {
    Initialize_State,
    Startup_State,
    FinalAscent_State,
    PreArmed_State,
    Armed_State,
    Fire_State,
    Landed_State,
    Error_State
} systemState_t;

// LEDs
#define RED_LED_ON() (LATC7 = 0)
#define RED_LED_OFF() (LATC7 = 1)
#define BLUE_LED_ON() (LATC6 = 0)
#define BLUE_LED_OFF() (LATC6 = 1)
#define WHITE_LED_ON() (LATC5 = 0)
#define WHITE_LED_OFF() (LATC5 = 1)
#define RED_LED_SET(val) (LATC7 = val)
#define BLUE_LED_SET(val) (LATC6 = val)
#define WHITE_LED_SET(val) (LATC5 = val)

// Buzzer
#define BUZZER_ON() (LATB4 = 1)
#define BUZZER_OFF() (LATB4 = 0)

// Altimeters
#define FIRE_A1() (LATC3 = 1)
#define UNFIRE_A1() (LATC3 = 0)
#define FIRE_A2() (LATC2 = 1)
#define UNFIRE_A2() (LATC2 = 0)

// Voltage Monitoring
#define ANALOG_SCALAR 3.72
// Current Monitorring
#define BATT_CURR_SCALAR 2.50 // mA/mV
#define CAN_CURR_SCALAR  0.25 // mA/mV NOTE this is not correct, it should be 0.05 but the board used for dev had an amp  with a gain of 20 instead of 100

void output_init(void);

void osc_init(void);

#endif	/* ARMING_H */

