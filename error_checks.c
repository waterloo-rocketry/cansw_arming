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

static uint32_t indicator_buzzer_last_millis = 0;
static bool buzzer_on = false;
void indicator_buzzer_heartbeat(void){

    int loop_time = millis() - indicator_buzzer_last_millis;

    if(buzzer_on == false && loop_time < 750 && battery1_active()){
        BUZZER_ON();
        buzzer_on = true;
    }

    else if(buzzer_on == true && loop_time >= 750 && loop_time < 1000){
        BUZZER_OFF();
        buzzer_on = false;
    }

    else if(buzzer_on == false && loop_time >= 1000 && loop_time < 1250 && battery2_active()){
        BUZZER_ON();
        buzzer_on = true;
    }

    else if(buzzer_on == true && loop_time >= 1250 && loop_time < 1500){
        BUZZER_OFF();
        buzzer_on = false;
    }

    else if(buzzer_on == false && loop_time >= 1500 && loop_time < 1750 && battery2_active()){
        BUZZER_ON();
        buzzer_on = true;
    }

    else if(buzzer_on == true && loop_time >= 1750 && loop_time < 2000){
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
