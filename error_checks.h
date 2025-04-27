#ifndef ERROR_CHECKS_H
#define ERROR_CHECKS_H

#include <stdbool.h>

#define MIN_BATTERY_THRESHOLD                                                                      \
    2000 // in mV, the minimum voltage to recognize a battery as plugged in
#define UNDERVOLTAGE_THRESHOLD 5000 // in mV
#define OVERVOLTAGE_THRESHOLD 10000 // in mV
#define BUS_OVERCURRENT_THRESHOLD 50 // in mA

bool check_battery_voltage_error(void);

bool check_bus_overcurrent_error(void);

void indicator_buzzer_heartbeat(void);

bool battery1_active(void);

bool battery2_active(void);

bool mag1_active(void);

bool mag2_active(void);

// reads the batt_curr for both batt lines from the ADC and applies it to two
// low pass filters must be run periodically to work correctly
void update_batt_curr_low_pass(void);

// returns the value from the higher cutoff frequency filter
double get_batt1_curr_low_pass(void);
double get_batt2_curr_low_pass(void);

// returns the value from the lower cut off frequency filter
double get_batt1_curr_low_low_pass(void);
double get_batt2_curr_low_low_pass(void);

#endif /* ERROR_CHECKS_H */

