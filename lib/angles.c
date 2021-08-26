#include <stdint.h>
#include <stdio.h>
#include <machine/fastmath.h>
#include <math.h>

#include "angles.h"
#include "main.h"
#include "lib/timer.h"
#include "util.h"
#include "lib/fixedptc.h"

#define SETUP_MEASUREMENTS 500


/** Data structures --------------------------------------------------------- */
typedef struct angles_axis_t {
	fixedpt x;
	fixedpt y;
	fixedpt z;
} angles_axis_t;

/** Static variables -------------------------------------------------------- */
static bool init;
static fixedpt pitch_avg = 0;
static fixedpt roll_avg = 0;
static fixedpt yaw_avg = 0;
static fixedpt gyro_x_avg = 0;
static fixedpt gyro_y_avg = 0;
static fixedpt gyro_z_avg = 0;
static fixedpt alpha = fixedpt_rconst(0.8);

static pthread_mutex_t init_mtx = PTHREAD_MUTEX_INITIALIZER;
static angles_axis_t gyro_err;
static angles_axis_t accel_err;
//static angles_axis_t gyro;
//static angles_axis_t accel;

static angles_t position;
static angles_t angle_vel;
static uint64_t last_time;
static pthread_mutex_t position_mtx = PTHREAD_MUTEX_INITIALIZER;

/** Function prototypes ----------------------------------------------------- */
//static void calculate_angles_gyro(void);
//static void calculate_angles_accel(void);

/** Function definitions ---------------------------------------------------- */
static angles_axis_t calculate_angles_gyro(void)
{
	mpu6050_gyro_t gyro_data = mpu6050_gyro_read();
	fixedpt gyro_x_fx = fixedpt_fromint(gyro_data.x);
	fixedpt gyro_y_fx = fixedpt_fromint(gyro_data.y);
	fixedpt gyro_z_fx = fixedpt_fromint(gyro_data.z);

	fixedpt x_dps = fixedpt_div(gyro_x_fx, fixedpt_rconst(32.8)) - gyro_err.x;
	fixedpt y_dps = fixedpt_div(gyro_y_fx, fixedpt_rconst(32.8)) - gyro_err.y;
	fixedpt z_dps = fixedpt_div(gyro_z_fx, fixedpt_rconst(32.8)) - gyro_err.z;

	uint64_t current_time = micros();
	fixedpt elapsed_time = fixedpt_rconst((current_time - last_time) / 1000000.0);
	last_time = current_time;

	angles_axis_t gyro;
	gyro.x = fixedpt_mul(x_dps, elapsed_time);
	gyro.y = fixedpt_mul(y_dps, elapsed_time);
	gyro.z = fixedpt_mul(z_dps, elapsed_time);
	return gyro;
}

static angles_axis_t calculate_angle_vel_gyro(void)
{
	mpu6050_gyro_t gyro_data = mpu6050_gyro_read();
	fixedpt gyro_x_fx = fixedpt_fromint(gyro_data.x);
	fixedpt gyro_y_fx = fixedpt_fromint(gyro_data.y);
	fixedpt gyro_z_fx = fixedpt_fromint(gyro_data.z);

	fixedpt x_dps = fixedpt_div(gyro_x_fx, fixedpt_rconst(32.8)) - gyro_err.x;
	fixedpt y_dps = fixedpt_div(gyro_y_fx, fixedpt_rconst(32.8)) - gyro_err.y;
	fixedpt z_dps = fixedpt_div(gyro_z_fx, fixedpt_rconst(32.8)) - gyro_err.z;

	angles_axis_t gyro;
	gyro.x = x_dps;
	gyro.y = y_dps;
	gyro.z = z_dps;
	return gyro;
}

static angles_axis_t calculate_angles_accel(void)
{
	mpu6050_accel_t accel_data = mpu6050_accel_read();
	double x = accel_data.x;
	double y = accel_data.y;
	double z = accel_data.z;
	angles_axis_t accel;

	accel.x = fixedpt_rconst (-(atan(x / (sqrt(pow(y, 2) + pow(z, 2)))) - accel_err.x)*180/M_PI);		//Converts the accelerometer data to angle position [degrees].
	accel.y = fixedpt_rconst( (atan(y / (sqrt(pow(x, 2) + pow(z, 2)))) - accel_err.y)*180/M_PI);
	accel.z = fixedpt_rconst( (atan(z / (sqrt(pow(y, 2) + pow(x, 2)))) - accel_err.z)*180/M_PI);
	return accel;
}

/** Public functions -------------------------------------------------------- */
void angles_init(void)
{
	pthread_mutex_lock(&print_mtx);
	printf("Calibrating MPU\n");
	pthread_mutex_unlock(&print_mtx);

	mpu6050_init();
	delay(202);

	int32_t gyro_x = 0;
	int32_t gyro_y = 0;
	int32_t gyro_z = 0;
	int32_t accel_x = 0;
	int32_t accel_y = 0;
	int32_t accel_z = 0;

	for (int i = 0; i < SETUP_MEASUREMENTS; i++) {
		mpu6050_gyro_t gyro_data = mpu6050_gyro_read();
		gyro_x = gyro_data.x + gyro_x;
		gyro_y = gyro_data.y + gyro_y;
		gyro_z = gyro_data.z + gyro_z;

		mpu6050_accel_t accel_data = mpu6050_accel_read();
		accel_x += accel_data.x;
		accel_y += accel_data.y;
		accel_z += accel_data.z;

		delay(10);
	}

	gyro_err.x = fixedpt_div(gyro_x, fixedpt_div(fixedpt_rconst(32.8), fixedpt_rconst(SETUP_MEASUREMENTS)));
	gyro_err.y = fixedpt_div(gyro_y, fixedpt_div(fixedpt_rconst(32.8), fixedpt_rconst(SETUP_MEASUREMENTS)));
	gyro_err.z = fixedpt_div(gyro_z, fixedpt_div(fixedpt_rconst(32.8), fixedpt_rconst(SETUP_MEASUREMENTS)));

	double x = accel_x / (double) SETUP_MEASUREMENTS;
	double y = accel_y / (double) SETUP_MEASUREMENTS;
	double z = accel_z / (double) SETUP_MEASUREMENTS;

	accel_err.x = fixedpt_rconst(atan(x / (sqrt(pow(y, 2) + pow(z, 2)))));
	accel_err.y = fixedpt_rconst(atan(y / (sqrt(pow(x, 2) + pow(z, 2)))));
	accel_err.z = fixedpt_rconst(atan(z / (sqrt(pow(y, 2) + pow(x, 2)))));

	pthread_mutex_lock(&print_mtx);
	printf("Gyro err: %.2f, %.2f, %.2f\n", fixedpt_2float(gyro_err.y), fixedpt_2float(gyro_err.x), fixedpt_2float(gyro_err.z));
	printf("Accel err: %.2f, %.2f, %.2f\n", fixedpt_2float(accel_err.z), fixedpt_2float(accel_err.y), fixedpt_2float(accel_err.z));
	pthread_mutex_unlock(&print_mtx);
	last_time = micros();

	pthread_mutex_lock(&init_mtx);
	init = true;
	pthread_mutex_unlock(&init_mtx);
}

void calculate_angles(void) {
	angles_axis_t gyro = calculate_angles_gyro();
	angles_axis_t gyro_vel = calculate_angle_vel_gyro();
	angles_axis_t accel = calculate_angles_accel();

	fixedpt pitch_acc = 0;
	fixedpt roll_acc = 0;
	fixedpt yaw_acc = 0;

	pthread_mutex_lock(&position_mtx);

	pitch_acc = fixedpt_mul(fixedpt_rconst(0.95), (position.pitch - gyro.y)) + fixedpt_mul(fixedpt_rconst(0.05), accel.x);
	roll_acc = fixedpt_mul(fixedpt_rconst(0.95), (position.roll - gyro.x)) + fixedpt_mul(fixedpt_rconst(0.05), accel.y);
	yaw_acc = fixedpt_mul(fixedpt_rconst(0.95), (position.yaw - gyro.z)) + fixedpt_mul(fixedpt_rconst(0.05), accel.z);

	pitch_avg = pitch_avg + fixedpt_mul(alpha, (pitch_acc - pitch_avg) );
	roll_avg = roll_avg + fixedpt_mul(alpha, (roll_acc - roll_avg) );
	yaw_avg = yaw_avg + fixedpt_mul(alpha, (yaw_acc - yaw_avg) );

	gyro_x_avg = gyro_x_avg + fixedpt_mul(alpha, fixedpt_sub(gyro_vel.x, gyro_x_avg));
	gyro_y_avg = gyro_y_avg + fixedpt_mul(alpha, fixedpt_sub(gyro_vel.y, gyro_y_avg));
	gyro_z_avg = gyro_z_avg + fixedpt_mul(alpha, fixedpt_sub(gyro_vel.z, gyro_z_avg));

	position.pitch = pitch_avg;
	position.roll = roll_avg;
	position.yaw = yaw_avg;
	
	angle_vel.pitch = gyro_y_avg;
	angle_vel.roll = gyro_x_avg;
	angle_vel.yaw = gyro_z_avg;

	if (fixedpt_2float( position.yaw) > 90)  position.yaw = fixedpt_rconst(90);		//We should set limits to it
	if (fixedpt_2float( position.yaw) < -90) position.yaw = fixedpt_rconst (-90);
	pthread_mutex_unlock(&position_mtx);
}

angles_t get_angles(void)
{
	pthread_mutex_lock(&position_mtx);
	angles_t aux = position;
	pthread_mutex_unlock(&position_mtx);
	return aux;
}

angles_t get_angle_vel(void)
{
	pthread_mutex_lock(&position_mtx);
	angles_t aux = angle_vel;
	pthread_mutex_unlock(&position_mtx);
	return aux;
}

bool angles_is_init(void)
{
	bool is_init;
	pthread_mutex_lock(&init_mtx);
	is_init = init;
	pthread_mutex_unlock(&init_mtx);
	return is_init;
}
