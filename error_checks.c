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

static uint32_t indicator_buzzer_last_millis = 0;
static bool buzzer_on = false;
void indicator_buzzer_heartbeat(void){

    int loop_time = millis() - indicator_buzzer_last_millis;

    if(buzzer_on == false && loop_time < 200 && mag1_active()){
        BUZZER_ON();
        buzzer_on = true;
    }

    else if(buzzer_on == true && loop_time >= 200 && loop_time < 1000){
        BUZZER_OFF();
        buzzer_on = false;
    }

    else if(buzzer_on == false && loop_time >= 1000 && loop_time < 1100 && mag2_active()){
        BUZZER_ON();
        buzzer_on = true;
    }

    else if(buzzer_on == true && loop_time >= 1100 && loop_time < 1500){
        BUZZER_OFF();
        buzzer_on = false;
    }

    else if(buzzer_on == false && loop_time >= 1500 && loop_time < 1600 && mag2_active()){
        BUZZER_ON();
        buzzer_on = true;
    }

    else if(buzzer_on == true && loop_time >= 1600 && loop_time < 2000){
        BUZZER_OFF();
        buzzer_on = false;
    }
    else if(loop_time >= 2000){
        indicator_buzzer_last_millis = millis();
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
#define alpha_low 0.95 
#define alpha_low_low 0.9975
double low_pass_curr = 0;
double low_low_pass_curr = 0;
double batt_curr_low_pass(void){
    double new_curr_reading = ADCC_GetSingleConversion(channel_BATT_CURR)*BATT_CURR_SCALAR;
    
    low_pass_curr = alpha_low*low_pass_curr + (1.0 - alpha_low)*new_curr_reading;
            
    low_low_pass_curr = alpha_low_low*low_low_pass_curr + (1.0 - alpha_low_low)*new_curr_reading;
    
    return low_pass_curr;
}

double get_batt_curr_low_low_pass(void){
    return low_low_pass_curr;
}
