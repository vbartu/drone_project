#include <stdint.h>
#include <stdio.h>
#include <machine/fastmath.h>
#include <math.h>

#include "angles.h"
#include "lib/timer.h"


#define SETUP_MEASUREMENTS 500


static double gyro_x_err;
static double gyro_y_err;
static double gyro_z_err;
static double accel_x_err;
static double accel_y_err;
static double accel_z_err;

static double x_angle;
static double y_angle;
static double z_angle;
static double x_angle2;
static double y_angle2;
static double z_angle2;

static uint64_t last_time;


void angles_init(void)
{
	mpu6050_init();
	delay(200);

	int16_t gyro_x = 0;
	int16_t gyro_y = 0;
	int16_t gyro_z = 0;
	double accel_x = 0;
	double accel_y = 0;
	double accel_z = 0;

	for (int i = 0; i < SETUP_MEASUREMENTS; i++) {
		mpu6050_gyro_t gyro = mpu6050_gyro_read();
		gyro_x += gyro.x;
		gyro_y += gyro.y;
		gyro_z += gyro.z;

		mpu6050_accel_t accel = mpu6050_accel_read();
		double x = accel.x;
		double y = accel.y;
		double z = accel.z;
		accel_x += atan(x / (sqrt(pow(y, 2) + pow(z, 2))));
		accel_y += atan(y / (sqrt(pow(x, 2) + pow(z, 2))));
		accel_z += atan(z / (sqrt(pow(y, 2) + pow(x, 2))));

		delay(10);
	}

	gyro_x_err = gyro_x / 32.8 / (double) SETUP_MEASUREMENTS;
	gyro_y_err = gyro_y / 32.8 / (double) SETUP_MEASUREMENTS;
	gyro_z_err = gyro_z / 32.8 / (double) SETUP_MEASUREMENTS;
	accel_x_err = accel_x / (double) SETUP_MEASUREMENTS;
	accel_y_err = accel_y / (double) SETUP_MEASUREMENTS;
	accel_z_err = accel_z / (double) SETUP_MEASUREMENTS;
	//accel_x_err *= 180.0 / M_PI;
	//accel_y_err *= 180.0 / M_PI;
	//accel_z_err *= 180.0 / M_PI;

	printf("Err: %.2f, %.2f, %.2f\n", gyro_y_err, gyro_x_err, gyro_z_err);
	printf("Err2: %.2f, %.2f, %.2f\n", accel_y_err, accel_x_err, accel_z_err);
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

void calculate_angles2(void)
{
	mpu6050_accel_t accel = mpu6050_accel_read();
	double x = accel.x;
	double y = accel.y;
	double z = accel.z;

	x_angle2 = atan(x / (sqrt(pow(y, 2) + pow(z, 2)))) - accel_x_err;
	y_angle2 = atan(y / (sqrt(pow(x, 2) + pow(z, 2)))) - accel_y_err;
	z_angle2 = atan(z / (sqrt(pow(y, 2) + pow(x, 2)))) - accel_z_err;
	x_angle2 *= 180 / M_PI;
	y_angle2 *= 180 / M_PI;
	z_angle2 *= 180 / M_PI;
}

angles_t get_angles(void)
{
	return (angles_t) {y_angle, x_angle, z_angle};
}

angles_t get_angles2(void)
{
	return (angles_t) {x_angle2, y_angle2, z_angle2};
}
