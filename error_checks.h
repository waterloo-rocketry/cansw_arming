#ifndef ERROR_CHECKS_H
#define	ERROR_CHECKS_H

#include <stdbool.h>

#define MIN_BATTERY_THRESHOLD 2000 // in mV, the minimum voltage to recognize a battery as plugged in
#define UNDERVOLTAGE_THRESHOLD 5000 // in mV
#define OVERVOLTAGE_THRESHOLD 10000 // in mV

bool check_battery_voltage_error(void);

void indicator_buzzer_heartbeat(void);

bool battery1_active(void);

bool battery2_active(void);

#endif	/* ERROR_CHECKS_H */

