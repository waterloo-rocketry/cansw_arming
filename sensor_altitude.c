#include <stdbool.h>
#include <stdint.h>
#include <math.h>

#include "sensor_altitude.h"

static uint16_t pressure = 0;
static bool new_pressure = false;

int32_t get_sensor_altitude(void) {
    new_pressure = false;
    // https://en.wikipedia.org/wiki/Pressure_altitude but pressure is in 10ths of a millibar
    double altitude = ((double)pressure) / 10132.5;
    altitude = 1 - pow(altitude, 0.190284);
    altitude = 145366.45 * altitude;
    return (uint32_t)altitude;
}

bool new_sensor_altitude_available(void){
    return new_pressure;
}

void parse_sensor_altitude(uint16_t can_pressure) {
    pressure = can_pressure;
    new_pressure = true;
}
