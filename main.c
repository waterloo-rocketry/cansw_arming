#include <stdio.h>
#include <stdlib.h>

#include "canlib/can.h"
#include "canlib/can_common.h"
#include "canlib/pic18f26k83/pic18f26k83_can.h"
#include "canlib/message_types.h"
#include "canlib/util/timing_util.h"
#include "canlib/util/can_tx_buffer.h"
#include "canlib/pic18f26k83/pic18f26k83_timer.h"

#include "setup.h"
#include "altitude_parsing.h"
#include "error_checks.h"
#include "mcc_generated_files/mcc.h"

#define _XTAL_FREQ 12000000 //12MHz

static void can_msg_handler(const can_msg_t *msg);
static void send_status_ok(void);

static const uint32_t field_asl = 1050;

typedef enum {
    Initialize_State,
    Startup_State,
    FinalAscent_State,
    Armed_State,
    Fire_State,
    Landed_State,
    Error_State
} systemState_t;

// Memory pool for CAN transmit buffer
uint8_t tx_pool[500];

int main(int argc, char** argv) {
    // init functions
    osc_init();
    timer0_init();
    output_init(); // initialize our outputs
    uart1_rx_init(9600, _XTAL_FREQ); // initialize the UART module
    // MCC generated functions
    ADCC_Initialize();
    FVR_Initialize();

    // enable global interupts
    INTCON0bits.GIE = 1;

    /***********set up CAN**********/

    // Set up CAN TX
    //TRISC0 = 0;
    //RC0PPS = 0x33;

    // Set up CAN RX
    //TRISC1 = 1;
    //ANSELC1 = 0;
    //CANRXPPS = 0x11;

    // set up CAN module
    //can_timing_t can_setup;
    //can_generate_timing_params(_XTAL_FREQ, &can_setup);

    //can_init(&can_setup, can_msg_handler);
    // set up CAN tx buffer
    //txb_init(tx_pool, sizeof(tx_pool), can_send, can_send_rdy);


    uint32_t last_millis = millis();
    uint32_t sensor_last_millis = millis();
    int32_t altitude = -999;
    uint8_t stage = 0;
    uint32_t main_deploy_time = 0;
    uint32_t last_altitude_time = millis();

    systemState_t systemState = Initialize_State;
    /***************Main Loop***************/
    while(1){

        CLRWDT(); //every loop resets the WatchDog Timer, it is set up to reset the board if we go more than 1 ms without reset

        //status updates every MAX_LOOP_TIME
        if(millis() >= last_millis + MAX_LOOP_TIME_DIFF_ms){
            last_millis = millis();

            // Toggle the white LED
            LATC5 = ~PORTCbits.RC5;
            
            // Toggle Blue and Red LEDs according to state
            
            if (stage == 4){
                LATC6 = ~PORTCbits.RC6;
            } else if (stage == 5) {
                LATC6 = ~PORTCbits.RC6;
                LATC7 = ~PORTCbits.RC7;
            }

            /***********Status Messages***********/

            //General Status Messages
            bool status_ok = true;
            status_ok &= check_battery_voltage_error();
            status_ok &= check_bus_overcurrent_error();
            //TODO: CHECK IF Watch Dog timer window violation has ocured
            if (status_ok){
               send_status_ok();
            }

            //Battery Status Messages
            can_msg_t bat_1_v_msg;
            build_analog_data_msg(millis(),
                                  SENSOR_ARM_BATT_1,
                                  (uint16_t)(ADCC_GetSingleConversion(channel_BATTERY_1)*ANALOG_SCALAR),
                                  &bat_1_v_msg);
            //txb_enqueue(&bat_1_v_msg);

            can_msg_t bat_2_v_msg;
            build_analog_data_msg(millis(),
                                  SENSOR_ARM_BATT_2,
                                  (uint16_t)(ADCC_GetSingleConversion(channel_BATTERY_2)*ANALOG_SCALAR),
                                  &bat_2_v_msg);
            //txb_enqueue(&bat_2_v_msg);

            //Mag Switch Voltage Messages
            can_msg_t mag_1_v_msg;
            build_analog_data_msg(millis(),
                                  SENSOR_MAG_1,
                                  (uint16_t)(ADCC_GetSingleConversion(channel_MAG_1)*ANALOG_SCALAR),
                                  &mag_1_v_msg);
            //txb_enqueue(&mag_1_v_msg);

            can_msg_t mag_2_v_msg;
            build_analog_data_msg(millis(),
                                  SENSOR_MAG_2,
                                  (uint16_t)(ADCC_GetSingleConversion(channel_MAG_2)*ANALOG_SCALAR),
                                  &mag_2_v_msg);
            //txb_enqueue(&mag_2_v_msg);

            // Current Messages
            can_msg_t batt_curr_msg;
            build_analog_data_msg(millis(),
                                  SENSOR_BATT_CURR,
                                  get_batt_curr_low_low_pass(),
                                  &batt_curr_msg);
            //txb_enqueue(&batt_curr_msg);

            can_msg_t bus_curr_msg;
            build_analog_data_msg(millis(),
                                  SENSOR_BUS_CURR,
                                  (uint16_t)(ADCC_GetSingleConversion(channel_CAN_CURR)*CAN_CURR_SCALAR),
                                  &bus_curr_msg);
            //txb_enqueue(&bus_curr_msg);

        }

        //high speed sensor checking
        if(millis() >= sensor_last_millis + MAX_SENSOR_LOOP_TIME_DIFF_ms){
            sensor_last_millis = millis();
            update_batt_curr_low_pass();
        }


        //handle altitude data, send message if new altitude received
        parse_altitude();
        if (new_altitude_available()){
            can_msg_t altitude_msg;
            build_altitude_data_msg(millis(), get_altitude(), &altitude_msg);
            txb_enqueue(&altitude_msg);
            altitude = get_altitude();
            last_altitude_time = millis();
        }

        if (altitude != -999 && millis() - last_altitude_time >= 10000){  // Not sure what we want this number to be
            systemState = Error_State;
        }

        switch (systemState){
            case Initialize_State:
            {
                if (mag1_active() == true /*|| mag2_active() == true*/){
                    systemState = Error_State;
                }

                else if (altitude != -999) {
                    systemState = Startup_State;
                    RED_LED_ON();
                }
            }
            break;
            case Startup_State:
            {
                stage = 1;
                if (mag1_active() == true /*|| mag2_active() == true*/){
                    systemState = Error_State;
                }

                else if (altitude >= 2000 + field_asl){
                    systemState = FinalAscent_State;
                    RED_LED_OFF();
                    BLUE_LED_ON();
                }
            }
            break;
            case FinalAscent_State:
            {
                stage = 2;
                if ((mag1_active() == true /*|| mag2_active() == true*/) && altitude < 3500 + field_asl){
                    systemState = Error_State;
                }
                else if (mag1_active() == true /*&& mag2_active() == true*/ && altitude >= 3500 + field_asl){
                    systemState = Armed_State;
                    RED_LED_ON();
                    BLUE_LED_ON();
                }
            }
            break;
            case Armed_State:
            {
                stage = 3;
                if (altitude <= 2500 + field_asl){
                    RED_LED_ON();
                    systemState = Fire_State;
                }
            }
            break;
            case Fire_State:
            {
                stage = 4;
                if (altitude <= 2000 + field_asl && main_deploy_time == 0){
                    ARM_A1(); //Fire Main Charge
                    main_deploy_time = millis();
                }else if (millis() - main_deploy_time >= 2000){
                    ARM_A2(); // Fire Backup Charge
                    systemState = Landed_State;
                }
            }
            break;
            case Landed_State:
            {
                stage = 1;
            }
            break;
            case Error_State:
            {
                stage = 5;
            }
            break;
        }


        // send queued messages
        //txb_heartbeat();

        //Mag-switch Arming Alert
        indicator_buzzer_heartbeat(stage);
    }
    RESET();
    // unreachable!
    return (EXIT_SUCCESS);
}

static void __interrupt() interrupt_handler(){
    if (PIE3bits.TMR0IE == 1 && PIR3bits.TMR0IF == 1) {
        timer0_handle_interrupt();
        PIR3bits.TMR0IF = 0;
    }
    if (PIR5) {
        can_handle_interrupt();
    }
    if(U1ERRIRbits.FERIF == 1 || U1ERRIRbits.RXFOIF){   // should probably do something if there is an error

    }
    else if (PIR3bits.U1RXIF == 1){
        uart1_handle_interrupt();
        PIR3bits.U1RXIF = 0;
    }
    else if(PIR3bits.U1EIF == 1){   // should probably do something if there is an error
        PIR3bits.U1EIF = 0;
    }

}

static void can_msg_handler(const can_msg_t *msg) {
    uint16_t msg_type = get_message_type(msg);

    // ignore messages that were sent from this board
    if (get_board_unique_id(msg) == BOARD_UNIQUE_ID) {
        return;
    }

    // declare this in advance cause we can't declare it inside the switch
    uint8_t alt_num = 0;
    enum ARM_STATE desired_arm_state = ARMED;

    switch (msg_type) {
        case MSG_LEDS_ON:
            RED_LED_ON();
            BLUE_LED_ON();
            WHITE_LED_ON();
            break;

        case MSG_LEDS_OFF:
            RED_LED_OFF();
            BLUE_LED_OFF();
            WHITE_LED_OFF();
            break;

        case MSG_RESET_CMD:
            RESET();

        default:
            // this is where we go for all the messages we don't care about
            break;
    }
}

// Send a CAN message with nominal status
static void send_status_ok(void) {
    can_msg_t board_stat_msg;
    build_board_stat_msg(millis(), E_NOMINAL, NULL, 0, &board_stat_msg);

    // send it off at low priority
    txb_enqueue(&board_stat_msg);
}
