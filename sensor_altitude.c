#include <stdbool.h>
#include <stdint.h>
#include <math.h>

#include "sensor_altitude.h"

static int32_t altitude = -999;
static bool new_altitude = false;

int32_t get_sensor_altitude(void) {
    new_altitude = false;
    return altitude;
}

bool new_sensor_altitude_available(void){
    return new_altitude;
}

void parse_sensor_altitude(uint16_t pressure) {
    // https://en.wikipedia.org/wiki/Pressure_altitude but pressure is in 10ths of a millibar
    double calced_altitude = ((double)pressure) / 10132.5;
    calced_altitude = 1 - pow(calced_altitude, 0.190284);
    calced_altitude = 145366.45 * calced_altitude;
    altitude = (uint32_t)calced_altitude;
    new_altitude = true;
}
