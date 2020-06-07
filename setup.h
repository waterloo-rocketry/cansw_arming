#ifndef ARMING_H
#define	ARMING_H

#include <xc.h>
#include "canlib/pic18f26k83/pic18f26k83_timer.h"
#include "mcc_generated_files/mcc.h"

// Time between main loop code execution
#define MAX_LOOP_TIME_DIFF_ms 1000
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD

#define UNDERVOLTAGE_THRESHOLD 5000 //in mV
<<<<<<< HEAD:setup.h

=======
        
>>>>>>> Finnish initial code for the mag switch state alert buzzer.:arming.h
=======
=======
>>>>>>> Begin code to use signal buzzer to indicate which mag switches are on.
#define UNDERVOLTAGE_THRESHOLD 5000 //in mV
    
>>>>>>> Begin code to use signal buzzer to indicate which mag switches are on.
=======

#define UNDERVOLTAGE_THRESHOLD 5000 //in mV
        
>>>>>>> Finnish initial code for the mag switch state alert buzzer.
=======

#define UNDERVOLTAGE_THRESHOLD 5000 //in mV
        
>>>>>>> Finnish initial code for the mag switch state alert buzzer.
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
#define ANALOG_SCALAR 3.72


  void output_init(void);

  void osc_init(void);
  
  void indicator_buzzer_heartbeat(void);
<<<<<<< HEAD

  void indicator_buzzer_heartbeat(void);


#ifdef	__cplusplus
}
#endif

=======
  
>>>>>>> Begin code to use signal buzzer to indicate which mag switches are on.
#endif	/* ARMING_H */

