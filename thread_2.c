#include <stdio.h>
#include <stdbool.h>

#include "main.h"
#include "lib/angles.h"
#include "lib/timer.h"
#include "lib/pid.h"

/** Static variables -------------------------------------------------------- */
static pid_ints_t pitch_pid;
static pid_ints_t roll_pid;
static pid_ints_t yaw_pid;
static volatile bool print_flag;

/** Function prototypes ----------------------------------------------------- */
static void print_flag_cb(void);

/** Function definitions ---------------------------------------------------- */
static void print_flag_cb(void)
{
	print_flag = true;
}

/** Public functions -------------------------------------------------------- */
void* thread_2_main(void* args)
{
	pthread_mutex_lock(&print_mtx);
	printf("Thread 2\n");
	pthread_mutex_unlock(&print_mtx);

	timer_init();

	app_timer_t* gyro_timer = create_timer(print_flag_cb, 500, 1000);

	while (!angles_is_init());

	pid_create(pitch_pid, 1.0, 0.0, 0.0, 0.0, 0.0);
	pid_create(roll_pid, 1.0, 0.0, 0.0, 0.0, 0.0);
	pid_create(yaw_pid, 1.0, 0.0, 0.0, 0.0, 0.0);

	while (1) {
		if (print_flag) {
			print_flag = false;
			angles_t angles = get_angles();

			double pitch_out = pid_fire(pitch_pid, 0, angles.pitch);
			double roll_out = pid_fire(roll_pid, 0, angles.roll);
			double yaw_out = pid_fire(yaw_pid, 0, angles.yaw);

			pthread_mutex_lock(&print_mtx);
			printf("PID: %.2f, %.2f, %.2f\n", pitch_out, roll_out, yaw_out);
			pthread_mutex_unlock(&print_mtx);
			delay(1500);
		}
	}
}
