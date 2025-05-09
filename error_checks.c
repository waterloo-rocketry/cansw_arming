#include "error_checks.h"
#include "setup.h"

#include "timer.h"

bool check_battery_voltage_error(void) {
    uint16_t batt1_voltage = (uint16_t)ADCC_GetSingleConversion(channel_BATTERY_1) * ANALOG_SCALAR;
    uint16_t batt2_voltage = (uint16_t)ADCC_GetSingleConversion(channel_BATTERY_2) * ANALOG_SCALAR;
    bool nominal = true; // keep track of if there was an error

    if (batt1_voltage > OVERVOLTAGE_THRESHOLD || batt2_voltage > OVERVOLTAGE_THRESHOLD) {
        uint8_t batt_data[2] = {0};
        uint16_t batt_voltage =
            batt1_voltage > batt2_voltage
                ? batt1_voltage
                : batt2_voltage; // since we have 2 batteries, send the voltage for the highest one
        batt_data[0] = (batt_voltage >> 8) & 0xff;
        batt_data[1] = (batt_voltage >> 0) & 0xff;

        nominal = false;
    }

    if (batt1_voltage < UNDERVOLTAGE_THRESHOLD || batt2_voltage < UNDERVOLTAGE_THRESHOLD) {
        uint8_t batt_data[2] = {0};
        uint16_t batt_voltage =
            batt1_voltage < batt2_voltage
                ? batt1_voltage
                : batt2_voltage; // since we have 2 batteries, send the voltage for the lowest one
        batt_data[0] = (batt_voltage >> 8) & 0xff;
        batt_data[1] = (batt_voltage >> 0) & 0xff;

        nominal = false;
    }

    return nominal;
}

bool check_bus_overcurrent_error(void) {
    uint16_t bus_curr = (uint16_t)(ADCC_GetSingleConversion(channel_CAN_CURR) * CAN_CURR_SCALAR);
    if (bus_curr > BUS_OVERCURRENT_THRESHOLD) {
        uint8_t curr_data[2] = {0};
        curr_data[0] = (bus_curr >> 8) & 0xff;
        curr_data[1] = (bus_curr >> 0) & 0xff;

        return false;
    }
    return true;
}

static uint32_t indicator_buzzer_last_millis = 0;
static bool buzzer_on = false;
void indicator_buzzer_heartbeat(void) {
    int loop_time = millis() - indicator_buzzer_last_millis;

    if (buzzer_on == false && loop_time < 200 && mag1_active()) {
        BUZZER_ON();
        buzzer_on = true;
    }

    else if (buzzer_on == true && loop_time >= 200 && loop_time < 1000) {
        BUZZER_OFF();
        buzzer_on = false;
    }

    else if (buzzer_on == false && loop_time >= 1000 && loop_time < 1100 && mag2_active()) {
        BUZZER_ON();
        buzzer_on = true;
    }

    else if (buzzer_on == true && loop_time >= 1100 && loop_time < 1500) {
        BUZZER_OFF();
        buzzer_on = false;
    }

    else if (buzzer_on == false && loop_time >= 1500 && loop_time < 1600 && mag2_active()) {
        BUZZER_ON();
        buzzer_on = true;
    }

    else if (buzzer_on == true && loop_time >= 1600 && loop_time < 2000) {
        BUZZER_OFF();
        buzzer_on = false;
    } else if (loop_time >= 2000) {
        indicator_buzzer_last_millis = millis();
    }
}

bool battery1_active(void) {
    return (uint16_t)ADCC_GetSingleConversion(channel_BATTERY_1) * ANALOG_SCALAR >
           MIN_BATTERY_THRESHOLD;
}

bool battery2_active(void) {
    return (uint16_t)ADCC_GetSingleConversion(channel_BATTERY_2) * ANALOG_SCALAR >
           MIN_BATTERY_THRESHOLD;
}

bool mag1_active(void) {
    return (uint16_t)ADCC_GetSingleConversion(channel_MAG_1) * ANALOG_SCALAR >
           MIN_BATTERY_THRESHOLD;
}

bool mag2_active(void) {
    return (uint16_t)ADCC_GetSingleConversion(channel_MAG_2) * ANALOG_SCALAR >
           MIN_BATTERY_THRESHOLD;
}

// zach derived the equation alpha = (Fs*T/5)/ 1 + (Fs*T/5)
//  where Fs = sampling frequency and T = response time
//  response time is equivalent to 5*tau or 5/2pi*Fc, where Fc is cutoff frequency

#define SAMPLE_FREQ (1000.0 / MAX_SENSOR_LOOP_TIME_DIFF_ms)
#define LOW_PASS_ALPHA(TR) ((SAMPLE_FREQ * TR / 5.0) / (1 + SAMPLE_FREQ * TR / 5.0))
#define LOW_PASS_RESPONSE_TIME 0.5 // seconds
#define LOW_LOW_PASS_RESPONSE_TIME 10 // seconds
double alpha_low = LOW_PASS_ALPHA(LOW_PASS_RESPONSE_TIME);
double alpha_low_low = LOW_PASS_ALPHA(LOW_LOW_PASS_RESPONSE_TIME);
double low_pass_curr1 = 0;
double low_low_pass_curr1 = 0;
double low_pass_curr2 = 0;
double low_low_pass_curr2 = 0;

void update_batt_curr_low_pass(void) {
    double new_curr1_reading = ADCC_GetSingleConversion(channel_BATT1_CURR) * BATT_CURR_SCALAR;
    double new_curr2_reading = ADCC_GetSingleConversion(channel_BATT2_CURR) * BATT_CURR_SCALAR;

    low_pass_curr1 = alpha_low * low_pass_curr1 + (1.0 - alpha_low) * new_curr1_reading;
    low_pass_curr2 = alpha_low * low_pass_curr2 + (1.0 - alpha_low) * new_curr2_reading;

    low_low_pass_curr1 =
        alpha_low_low * low_low_pass_curr1 + (1.0 - alpha_low_low) * new_curr1_reading;
    low_low_pass_curr2 =
        alpha_low_low * low_low_pass_curr2 + (1.0 - alpha_low_low) * new_curr2_reading;
}

double get_batt1_curr_low_pass(void) {
    return low_pass_curr1;
}

double get_batt2_curr_low_pass(void) {
    return low_pass_curr2;
}

double get_batt1_curr_low_low_pass(void) {
    return low_low_pass_curr1;
}

double get_batt2_curr_low_low_pass(void) {
    return low_low_pass_curr2;
}
