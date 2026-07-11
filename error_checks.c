#include "error_checks.h"
#include "setup.h"

#include "timer.h"

bool check_battery_voltage_overvoltage(void) {
    uint16_t batt1_voltage = (uint16_t)ADCC_GetSingleConversion(channel_BATTERY_1) * ANALOG_SCALAR;
    // uint16_t batt2_voltage = (uint16_t)ADCC_GetSingleConversion(channel_BATTERY_2) * ANALOG_SCALAR;
    bool over_voltage = false; // keep track of if there was an error

    if (batt1_voltage > OVERVOLTAGE_THRESHOLD) {
        uint8_t batt_data[2] = {0};
        batt_data[0] = (batt1_voltage >> 8) & 0xff;
        batt_data[1] = (batt1_voltage >> 0) & 0xff;

        over_voltage = true;
    }

    return over_voltage;
}

bool check_battery_voltage_undervoltage(void) {
    uint16_t batt1_voltage = (uint16_t)ADCC_GetSingleConversion(channel_BATTERY_1) * ANALOG_SCALAR;
    bool under_voltage = false; // keep track of if there was an error

    if (batt1_voltage < UNDERVOLTAGE_THRESHOLD) {
        uint8_t batt_data[2] = {0};
        batt_data[0] = (batt1_voltage >> 8) & 0xff;
        batt_data[1] = (batt1_voltage >> 0) & 0xff;

        under_voltage = true;
    }

    return under_voltage;
}

bool check_bus_overcurrent_healthy(void) {
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

    if (buzzer_on == false && loop_time < 200 && battery1_active()) {
        BUZZER_ON();
        buzzer_on = true;
    } else if ((true == buzzer_on) && (loop_time >= 200) && (loop_time < 1000) && (mag1_active())) {
        BUZZER_ON();
        buzzer_on = true;
    } else if (buzzer_on == true && loop_time >= 200 && loop_time < 2000) {
        BUZZER_OFF();
        buzzer_on = false;
    } else if (loop_time >= 2000) {
        indicator_buzzer_last_millis = millis();
    } else {
        BUZZER_OFF();
        buzzer_on = false;
    }
}

bool battery1_active(void) {
    return (uint16_t)ADCC_GetSingleConversion(channel_BATTERY_1) * ANALOG_SCALAR >
           MIN_BATTERY_THRESHOLD;
}

bool battery2_active(void) {
    // return (uint16_t)ADCC_GetSingleConversion(channel_BATTERY_2) * ANALOG_SCALAR >
    //        MIN_BATTERY_THRESHOLD;
    return false;
}

bool mag1_active(void) {
    return (uint16_t)ADCC_GetSingleConversion(channel_MAG_1) * ANALOG_SCALAR >
           MIN_BATTERY_THRESHOLD;
}

bool mag2_active(void) {
    // return (uint16_t)ADCC_GetSingleConversion(channel_MAG_2) * ANALOG_SCALAR >
    //        MIN_BATTERY_THRESHOLD;
    return false;
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

    low_pass_curr1 = alpha_low * low_pass_curr1 + (1.0 - alpha_low) * new_curr1_reading;

    low_low_pass_curr1 =
        alpha_low_low * low_low_pass_curr1 + (1.0 - alpha_low_low) * new_curr1_reading;
}

double get_batt1_curr_low_pass(void) {
    return low_pass_curr1;
}

double get_batt2_curr_low_pass(void) {
    return 0;
}

double get_batt1_curr_low_low_pass(void) {
    return low_low_pass_curr1;
}

double get_batt2_curr_low_low_pass(void) {
    return 0;
}
