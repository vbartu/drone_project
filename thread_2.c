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
static bool print_flag;

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

	pid_create(pitch_pid, 1.0, 0.0, 0.0, 0.0, 0.0);
	pid_create(roll_pid, 1.0, 0.0, 0.0, 0.0, 0.0);
	pid_create(yaw_pid, 1.0, 0.0, 0.0, 0.0, 0.0);

	while (1) {
			angles_t gy = get_angles_gyro();

			double pitch_out = pid_fire(pitch_pid, 0, gy.pitch);
			double roll_out = pid_fire(roll_pid, 0, gy.roll);
			double yaw_out = pid_fire(yaw_pid, 0, gy.yaw);

			pthread_mutex_lock(&print_mtx);
			printf("PID: %.2f, %.2f, %.2f\n", pitch_out, roll_out, yaw_out);
			pthread_mutex_unlock(&print_mtx);
			delay(500);
	}
}
