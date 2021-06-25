#ifndef __ANGLES_H
#define __ANGLES_H

#include <stdbool.h>

#include "sensors/mpu6050.h"


typedef struct angles_t {
	double pitch;
	double roll;
	double yaw;
} angles_t;


void angles_init(void);

void calculate_angles_gyro(void);
void calculate_angles_accel(void);

angles_t get_angles_gyro(void);
angles_t get_angles_accel(void);

bool angles_is_init(void);

#endif // __ANGLES_H
