#include <stdint.h>
#include <stdio.h>
#include <machine/fastmath.h>
#include <math.h>

#include "angles.h"
#include "main.h"
#include "lib/timer.h"


#define SETUP_MEASUREMENTS 300

/** Static variables -------------------------------------------------------- */
static bool init;
static pthread_mutex_t init_mtx = PTHREAD_MUTEX_INITIALIZER;
static double gyro_x_err;
static double gyro_y_err;
static double gyro_z_err;
static double accel_x_err;
static double accel_y_err;
static double accel_z_err;
static double x_angle_gyro;
static double y_angle_gyro;
static double z_angle_gyro;
static double x_angle_accel;
static double y_angle_accel;
static double z_angle_accel;
static uint64_t last_time;
static pthread_mutex_t gyro_mtx = PTHREAD_MUTEX_INITIALIZER;

/** Function prototypes ----------------------------------------------------- */
/** Function definitions ---------------------------------------------------- */
/** Public functions -------------------------------------------------------- */
void angles_init(void)
{
	pthread_mutex_lock(&print_mtx);
	printf("Calibrating MPU\n");
	pthread_mutex_unlock(&print_mtx);

	mpu6050_init();
	delay(50);

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


	pthread_mutex_lock(&print_mtx);
	printf("Gyro err: %.2f, %.2f, %.2f\n", gyro_y_err, gyro_x_err, gyro_z_err);
	printf("Accel err: %.2f, %.2f, %.2f\n", accel_x_err, accel_y_err,
			accel_z_err);
	pthread_mutex_unlock(&print_mtx);
	last_time = micros();

	pthread_mutex_lock(&init_mtx);
	init = true;
	pthread_mutex_unlock(&init_mtx);
}

void calculate_angles_gyro(void)
{
	mpu6050_gyro_t gyro = mpu6050_gyro_read();
	double x_dps = gyro.x / 32.8 - gyro_x_err;
	double y_dps = gyro.y / 32.8 - gyro_y_err;
	double z_dps = gyro.z / 32.8 - gyro_z_err;

	uint64_t current_time = micros();
	double elapsed_time = (current_time - last_time) / 1000000.0;
	last_time = current_time;

	pthread_mutex_lock(&gyro_mtx);
	x_angle_gyro += x_dps * elapsed_time;
	y_angle_gyro += y_dps * elapsed_time;
	z_angle_gyro += z_dps * elapsed_time;
	pthread_mutex_unlock(&gyro_mtx);
}

void calculate_angles_accel(void)
{
	mpu6050_accel_t accel = mpu6050_accel_read();
	double x = accel.x;
	double y = accel.y;
	double z = accel.z;

	x_angle_accel = atan(x / (sqrt(pow(y, 2) + pow(z, 2)))) - accel_x_err;
	y_angle_accel = atan(y / (sqrt(pow(x, 2) + pow(z, 2)))) - accel_y_err;
	z_angle_accel = atan(z / (sqrt(pow(y, 2) + pow(x, 2)))) - accel_z_err;
	x_angle_accel *= 180 / M_PI;
	y_angle_accel *= 180 / M_PI;
	z_angle_accel *= 180 / M_PI;
}

angles_t get_angles_gyro(void)
{
	pthread_mutex_lock(&gyro_mtx);
	angles_t angles = {y_angle_gyro, x_angle_gyro, z_angle_gyro};
	pthread_mutex_unlock(&gyro_mtx);
	return angles;
}

angles_t get_angles_accel(void)
{
	return (angles_t) {x_angle_accel, y_angle_accel, z_angle_accel};
}

bool angles_is_init(void)
{
	bool is_init;
	pthread_mutex_lock(&init_mtx);
	is_init = init;
	pthread_mutex_unlock(&init_mtx);
	return is_init;
}
