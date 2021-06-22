#include <stdio.h>
#include <stdbool.h>

#include "main.h"
#include "lib/angles.h"
#include "lib/timer.h"

/** Static variables -------------------------------------------------------- */
static bool read_gyro_flag;
static bool read_accel_flag;
static bool print_flag;

/** Function prototypes ----------------------------------------------------- */
static void read_gyro_flag_cb(void);
static void read_accel_flag_cb(void);
static void print_flag_cb(void);

/** Function definitions ---------------------------------------------------- */
static void read_gyro_flag_cb(void)
{
	read_gyro_flag = true;
}

static void read_accel_flag_cb(void)
{
	read_accel_flag = true;
}

static void print_flag_cb(void)
{
	print_flag = true;
}

/** Public functions -------------------------------------------------------- */
void* thread_1_main(void* args)
{
	angles_init();
	timer_init();
	app_timer_t* gyro_timer = create_timer(read_gyro_flag_cb, 500, 50);
	app_timer_t* accel_timer = create_timer(read_accel_flag_cb, 500, 200);
	//app_timer_t* print_timer = create_timer(print_flag_cb, 500, 1000);

	while (1) {
		if (read_gyro_flag) {
			read_gyro_flag = false;
			calculate_angles_gyro();
		}

		if (read_accel_flag) {
			read_accel_flag = false;
			calculate_angles_accel();
		}

		if (print_flag) {
			print_flag = false;
			angles_t gy = get_angles_gyro();
			angles_t acc = get_angles_accel();
			//pthread_mutex_lock(&print_mtx);
			printf("Gyro: %.2f, %.2f, %.2f\n", gy.pitch, gy.roll, gy.yaw);
			printf("Acc: %.2f, %.2f, %.2f\n", acc.pitch, acc.roll, acc.yaw);
			//pthread_mutex_unlock(&print_mtx);
		}
	}
}
