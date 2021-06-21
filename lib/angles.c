#include <stdint.h>
#include <stdio.h>

#include "angles.h"
#include "lib/timer.h"


#define SETUP_MEASUREMENTS 500


static double gyro_x_err;
static double gyro_y_err;
static double gyro_z_err;

static double x_angle;
static double y_angle;
static double z_angle;

static uint64_t last_time;


void angles_init(void)
{
	mpu6050_init();
	delay(200);

	int16_t gyro_x = 0;
	int16_t gyro_y = 0;
	int16_t gyro_z = 0;

	for (int i = 0; i < SETUP_MEASUREMENTS; i++) {
		mpu6050_gyro_t gyro = mpu6050_gyro_read();
		gyro_x += gyro.x;
		gyro_y += gyro.y;
		gyro_z += gyro.z;
		delay(20);
	}

	gyro_x_err = gyro_x / 32.8 / (double) SETUP_MEASUREMENTS;
	gyro_y_err = gyro_y / 32.8 / (double) SETUP_MEASUREMENTS;
	gyro_z_err = gyro_z / 32.8 / (double) SETUP_MEASUREMENTS;

	printf("Err: %.2f, %.2f, %.2f\n", gyro_y_err, gyro_x_err, gyro_z_err);
	last_time = micros();
}


void calculate_angles(void)
{
	mpu6050_gyro_t gyro = mpu6050_gyro_read();
	double x_dps = gyro.x / 32.8 - gyro_x_err;
	double y_dps = gyro.y / 32.8 - gyro_y_err;
	double z_dps = gyro.z / 32.8 - gyro_z_err;

	uint64_t current_time = micros();
	double elapsed_time = (current_time - last_time) / 1000000.0;
	last_time = current_time;

	x_angle += x_dps * elapsed_time;
	y_angle += y_dps * elapsed_time;
	z_angle += z_dps * elapsed_time;
}

angles_t get_angles(void)
{
	return (angles_t) {y_angle, x_angle, z_angle};
}
