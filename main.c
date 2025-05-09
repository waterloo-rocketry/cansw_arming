#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "canlib/canlib.h"
#include "timer.h"

#include "altitude_parsing.h"
#include "error_checks.h"
#include "mcc_generated_files/mcc.h"
#include "setup.h"

#define _XTAL_FREQ 12000000 // 12MHz

static void can_msg_handler(const can_msg_t *msg);

// Altimeter 1
static can_alt_arm_state_t alt_stratologger_arm_state = ALT_ARM_STATE_ARMED;

// Altimeter 2
static can_alt_arm_state_t alt_raven_arm_state = ALT_ARM_STATE_ARMED;

// Memory pool for CAN transmit buffer
uint8_t tx_pool[500];

int main(int argc, char **argv) {
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
    TRISC0 = 0;
    RC0PPS = 0x33;

    // Set up CAN RX
    TRISC1 = 1;
    ANSELC1 = 0;
    CANRXPPS = 0x11;

    // set up CAN module
    can_timing_t can_setup;
    can_generate_timing_params(_XTAL_FREQ, &can_setup);

    can_init(&can_setup, can_msg_handler);
    // set up CAN tx buffer
    txb_init(tx_pool, sizeof(tx_pool), can_send, can_send_rdy);

    uint32_t last_millis = millis();
    uint32_t sensor_last_millis = millis();

    /***************Main Loop***************/
    while (1) {
        CLRWDT(); // every loop resets the WatchDog Timer, it is set up to reset the board if we go
                  // more than 1 ms without reset

        // status updates every MAX_LOOP_TIME
        if (millis() >= last_millis + MAX_LOOP_TIME_DIFF_ms) {
            last_millis = millis();

            // Toggle the white LED
            LATC5 = ~PORTCbits.RC5;

            /***********Status Messages***********/

            // General Status Messages
            uint32_t status_bitfield = 0;
            if (!check_battery_voltage_error()) {
                status_bitfield |= (1 << E_12V_OVER_VOLTAGE_OFFSET);
                // FIXME: replace with a more approiate message
            }
            if (!check_bus_overcurrent_error()) {
                status_bitfield |= (1 << E_5V_OVER_CURRENT_OFFSET);
            }
            // TODO: CHECK IF Watch Dog timer window violation has ocured
            can_msg_t board_stat_msg;
            build_general_board_status_msg(
                PRIO_MEDIUM, millis(), status_bitfield, 0, &board_stat_msg
            );
            txb_enqueue(&board_stat_msg);

            // Altimeter Status Messages
            can_msg_t alt_stratologger_arm_stat_msg;
            build_alt_arm_status_msg(
                PRIO_HIGH,
                millis(),
                ALTIMETER_STRATOLOGGER,
                alt_stratologger_arm_state,
                (uint16_t)(ADCC_GetSingleConversion(channel_A1_DROGUE) * ANALOG_SCALAR),
                (uint16_t)(ADCC_GetSingleConversion(channel_A1_MAIN) * ANALOG_SCALAR),
                &alt_stratologger_arm_stat_msg
            );
            txb_enqueue(&alt_stratologger_arm_stat_msg);

            can_msg_t alt_raven_arm_stat_msg;
            build_alt_arm_status_msg(
                PRIO_HIGH,
                millis(),
                ALTIMETER_RAVEN,
                alt_raven_arm_state,
                (uint16_t)(ADCC_GetSingleConversion(channel_A2_DROGUE) * ANALOG_SCALAR),
                (uint16_t)(ADCC_GetSingleConversion(channel_A2_MAIN) * ANALOG_SCALAR),
                &alt_raven_arm_stat_msg
            );
            txb_enqueue(&alt_raven_arm_stat_msg);

            // Battery Status Messages
            can_msg_t bat_1_v_msg;
            build_analog_data_msg(
                PRIO_MEDIUM,
                millis(),
                SENSOR_RA_BATT_VOLT_1,
                (uint16_t)(ADCC_GetSingleConversion(channel_BATTERY_1) * ANALOG_SCALAR),
                &bat_1_v_msg
            );
            txb_enqueue(&bat_1_v_msg);

            can_msg_t bat_2_v_msg;
            build_analog_data_msg(
                PRIO_MEDIUM,
                millis(),
                SENSOR_RA_BATT_VOLT_2,
                (uint16_t)(ADCC_GetSingleConversion(channel_BATTERY_2) * ANALOG_SCALAR),
                &bat_2_v_msg
            );
            txb_enqueue(&bat_2_v_msg);

            // Mag Switch Voltage Messages
            can_msg_t mag_1_v_msg;
            build_analog_data_msg(
                PRIO_MEDIUM,
                millis(),
                SENSOR_RA_MAG_VOLT_1,
                (uint16_t)(ADCC_GetSingleConversion(channel_MAG_1) * ANALOG_SCALAR),
                &mag_1_v_msg
            );
            txb_enqueue(&mag_1_v_msg);

            can_msg_t mag_2_v_msg;
            build_analog_data_msg(
                PRIO_MEDIUM,
                millis(),
                SENSOR_RA_MAG_VOLT_2,
                (uint16_t)(ADCC_GetSingleConversion(channel_MAG_2) * ANALOG_SCALAR),
                &mag_2_v_msg
            );
            txb_enqueue(&mag_2_v_msg);

            // Current Messages
            can_msg_t batt1_curr_msg;
            build_analog_data_msg(
                PRIO_MEDIUM,
                millis(),
                SENSOR_RA_BATT_CURR_1,
                get_batt1_curr_low_low_pass(),
                &batt1_curr_msg
            );
            txb_enqueue(&batt1_curr_msg);

            can_msg_t batt2_curr_msg;
            build_analog_data_msg(
                PRIO_MEDIUM,
                millis(),
                SENSOR_RA_BATT_CURR_2,
                get_batt2_curr_low_low_pass(),
                &batt2_curr_msg
            );
            txb_enqueue(&batt2_curr_msg);

            can_msg_t bus_curr_msg;
            build_analog_data_msg(
                PRIO_MEDIUM,
                millis(),
                SENSOR_5V_CURR,
                (uint16_t)(ADCC_GetSingleConversion(channel_CAN_CURR) * CAN_CURR_SCALAR),
                &bus_curr_msg
            );
            txb_enqueue(&bus_curr_msg);
        }

        // high speed sensor checking
        if (millis() >= sensor_last_millis + MAX_SENSOR_LOOP_TIME_DIFF_ms) {
            sensor_last_millis = millis();
            update_batt_curr_low_pass();
        }

        // handle altitude data, send message if new altitude received
        parse_altitude();
        if (new_altitude_available()) {
            can_msg_t altitude_msg;
            build_altitude_data_msg(PRIO_HIGH, millis(), get_altitude(), &altitude_msg);
            txb_enqueue(&altitude_msg);
        }

        if (new_velocity_available()) {
            can_msg_t velocity_msg;
            build_analog_data_msg(
                PRIO_HIGH, millis(), SENSOR_VELOCITY, get_velocity(), &velocity_msg
            );
            txb_enqueue(&velocity_msg);
        }
        // set io to arm state of Stratologger
        if (alt_stratologger_arm_state == ALT_ARM_STATE_DISARMED) {
            DISARM_A1();
            RED_LED_OFF();
        } else {
            ARM_A1();
            RED_LED_ON();
        }

        // set io to arm state of Raven
        if (alt_raven_arm_state == ALT_ARM_STATE_DISARMED) {
            DISARM_A2();
            BLUE_LED_OFF();
        } else {
            ARM_A2();
            BLUE_LED_ON();
        }

        // send queued messages
        txb_heartbeat();

        // Mag-switch Arming Alert
        indicator_buzzer_heartbeat();
    }
    RESET();
    // unreachable!
    return (EXIT_SUCCESS);
}

static void __interrupt() interrupt_handler() {
    if (PIE3bits.TMR0IE == 1 && PIR3bits.TMR0IF == 1) {
        timer0_handle_interrupt();
        PIR3bits.TMR0IF = 0;
    }
    if (PIR5) {
        can_handle_interrupt();
    }
    if (U1ERRIRbits.FERIF == 1) { // If we have a framing error, discard the byte
        char garbage = U1RXB; // read the next byte in the buffer to clear the garbage
    } else if (PIR3bits.U1RXIF == 1) {
        uart1_handle_interrupt();
        PIR3bits.U1RXIF = 0;
    } else if (PIR3bits.U1EIF == 1) { // should probably do something if there is an error
        PIR3bits.U1EIF = 0;
    } else if (U1ERRIRbits.RXFOIF == 1) { // should probably do something if the FIFO is overflowing
        U1ERRIRbits.RXFOIF = 0;
    }
}

static void can_msg_handler(const can_msg_t *msg) {
    uint16_t msg_type = get_message_type(msg);

    // ignore messages that were sent from this board
    if ((get_board_type_unique_id(msg) == BOARD_TYPE_UNIQUE_ID) &&
        (get_board_inst_unique_id(msg) == BOARD_INST_UNIQUE_ID)) {
        return;
    }

    // declare this in advance cause we can't declare it inside the switch
    can_altimeter_id_t alt_id = 0;
    can_alt_arm_state_t desired_arm_state = ALT_ARM_STATE_ARMED;
    uint8_t dest_board_type_id, dest_board_inst_id; // the destination board id for reset

    switch (msg_type) {
        case MSG_ALT_ARM_CMD:
            get_alt_arm_state(msg, &alt_id, &desired_arm_state);
            if (alt_id == ALTIMETER_RAVEN) {
                alt_raven_arm_state = desired_arm_state;
            }
            if (alt_id == ALTIMETER_STRATOLOGGER) {
                alt_stratologger_arm_state = desired_arm_state;
            }
            break;

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
            if (check_board_need_reset(msg)) {
                RESET();
            }
            break;

        default:
            // this is where we go for all the messages we don't care about
            break;
    }
}
