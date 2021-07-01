#ifndef SENSOR_ALTITUDE_H
#define	SENSOR_ALTITUDE_H

#ifdef	__cplusplus
extern "C" {
#endif

int32_t get_sensor_altitude(void);

bool new_sensor_altitude_available(void);

void parse_sensor_altitude(uint16_t pressure);

#ifdef	__cplusplus
}
#endif

#endif	/* SENSOR_ALTITUDE_H */

