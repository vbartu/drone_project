#ifndef __ANGLES_H
#define __ANGLES_H

#include "sensors/mpu6050.h"


typedef struct angles_t {
	double pitch;
	double roll;
	double yaw;
} angles_t;


void angles_init(void);

void calculate_angles(void);

angles_t get_angles(void);

#endif // __ANGLES_H
