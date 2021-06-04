#include "error_checks.h"
#include "setup.h"

#include "canlib/can.h"
#include "canlib/can_common.h"
#include "canlib/pic18f26k83/pic18f26k83_can.h"
#include "canlib/message_types.h"
#include "canlib/util/can_tx_buffer.h"


bool check_battery_voltage_error(void){
    uint16_t batt1_voltage = (uint16_t)ADCC_GetSingleConversion(channel_BATTERY_1)*ANALOG_SCALAR;
    uint16_t batt2_voltage = (uint16_t)ADCC_GetSingleConversion(channel_BATTERY_2)*ANALOG_SCALAR;
    bool nominal = true; // keep track of if there was an error

    if(batt1_voltage > OVERVOLTAGE_THRESHOLD || batt2_voltage > OVERVOLTAGE_THRESHOLD){
        uint8_t batt_data[2] = {0};
        uint16_t batt_voltage = batt1_voltage > batt2_voltage ? batt1_voltage : batt2_voltage; // since we have 2 batteries, send the voltage for the highest one
        batt_data[0] = (batt_voltage >> 8) & 0xff;
        batt_data[1] = (batt_voltage >> 0) & 0xff;

        can_msg_t error_msg;
        build_board_stat_msg(millis(), E_BATT_OVER_VOLTAGE, batt_data, 2, &error_msg);
        txb_enqueue(&error_msg);

        nominal = false;
    }

    if (batt1_voltage < UNDERVOLTAGE_THRESHOLD || batt2_voltage < UNDERVOLTAGE_THRESHOLD){
        uint8_t batt_data[2] = {0};
        uint16_t batt_voltage = batt1_voltage < batt2_voltage ? batt1_voltage : batt2_voltage; // since we have 2 batteries, send the voltage for the lowest one
        batt_data[0] = (batt_voltage >> 8) & 0xff;
        batt_data[1] = (batt_voltage >> 0) & 0xff;

        can_msg_t error_msg;
        build_board_stat_msg(millis(), E_BATT_UNDER_VOLTAGE, batt_data, 2, &error_msg);
        txb_enqueue(&error_msg);

        nominal = false;
    }

    return nominal;
}

bool check_bus_overcurrent_error(void){
    uint16_t bus_curr = (uint16_t)(ADCC_GetSingleConversion(channel_CAN_CURR)*CAN_CURR_SCALAR);
    if(bus_curr > BUS_OVERCURRENT_THRESHOLD){
        uint8_t curr_data[2] = {0};
        curr_data[0] = (bus_curr >> 8) & 0xff;
        curr_data[1] = (bus_curr >> 0) & 0xff;

        can_msg_t error_msg;
        build_board_stat_msg(millis(), E_BUS_OVER_CURRENT, curr_data, 2, &error_msg);
        txb_enqueue(&error_msg);

        return false;
    }
    return true;
}

static uint32_t indicator_led_last_millis = 0;
bool flash_state = false;
void indicator_led_heartbeat(systemState_t state){
    /*
    R B W  leds, can be on (Y), off (N) or flashing (F)
    N N Y  initialize
    N Y Y  startup
    N F F  finalascent
    F N N  armed
    Y N N  fire
    N Y N  landed
    F F F  error
    */
    if (millis() - indicator_led_last_millis > 250) {
        flash_state = !flash_state;
    }
    switch (state) {
        case Initialize_State:
            RED_LED_OFF();
            BLUE_LED_OFF();
            WHITE_LED_ON();
            break;
        case Startup_State:
            RED_LED_OFF();
            BLUE_LED_ON();
            WHITE_LED_ON();
            break;
        case FinalAscent_State:
            RED_LED_OFF();
            BLUE_LED_SET(flash_state);
            WHITE_LED_SET(flash_state);
            break;
        case Armed_State:
            RED_LED_SET(flash_state);
            BLUE_LED_OFF();
            WHITE_LED_OFF();
            break;
        case Fire_State:
            RED_LED_ON();
            BLUE_LED_OFF();
            WHITE_LED_OFF();
            break;
        case Landed_State:
            RED_LED_OFF();
            BLUE_LED_ON();
            WHITE_LED_OFF();
            break;
        case Error_State:
            RED_LED_SET(flash_state);
            BLUE_LED_SET(flash_state);
            WHITE_LED_SET(flash_state);
            break;
    }
}

bool battery1_active(void){
    return (uint16_t)ADCC_GetSingleConversion(channel_BATTERY_1)*ANALOG_SCALAR > MIN_BATTERY_THRESHOLD;
}

bool battery2_active(void){
    return (uint16_t)ADCC_GetSingleConversion(channel_BATTERY_2)*ANALOG_SCALAR > MIN_BATTERY_THRESHOLD;
}

bool mag1_active(void){
    return (uint16_t)ADCC_GetSingleConversion(channel_MAG_1)*ANALOG_SCALAR > MIN_BATTERY_THRESHOLD;
}

bool mag2_active(void){
    return (uint16_t)ADCC_GetSingleConversion(channel_MAG_2)*ANALOG_SCALAR > MIN_BATTERY_THRESHOLD;
}



//zach derived the equation alpha = (Fs*T/5)/ 1 + (Fs*T/5)
// where Fs = sampling frequency and T = response time
// response time is equivalent to 5*tau or 5/2pi*Fc, where Fc is cutoff frequency

#define SAMPLE_FREQ (1000.0 / MAX_SENSOR_LOOP_TIME_DIFF_ms)
#define LOW_PASS_ALPHA(TR) ((SAMPLE_FREQ * TR / 5.0) / (1 + SAMPLE_FREQ * TR / 5.0))
#define LOW_PASS_RESPONSE_TIME 0.5  //seconds
#define LOW_LOW_PASS_RESPONSE_TIME 10  //seconds
double alpha_low = LOW_PASS_ALPHA(LOW_PASS_RESPONSE_TIME);
double alpha_low_low = LOW_PASS_ALPHA(LOW_LOW_PASS_RESPONSE_TIME);
double low_pass_curr = 0;
double low_low_pass_curr = 0;
void update_batt_curr_low_pass(void){
    double new_curr_reading = ADCC_GetSingleConversion(channel_BATT_CURR)*BATT_CURR_SCALAR;

    low_pass_curr = alpha_low*low_pass_curr + (1.0 - alpha_low)*new_curr_reading;

    low_low_pass_curr = alpha_low_low*low_low_pass_curr + (1.0 - alpha_low_low)*new_curr_reading;

}

double get_batt_curr_low_pass(void){
    return low_pass_curr;
}

double get_batt_curr_low_low_pass(void){
    return low_low_pass_curr;
}
