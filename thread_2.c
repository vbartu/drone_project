#include <stdio.h>
#include <stdbool.h>

#include "main.h"
#include "lib/angles.h"
#include "lib/timer.h"
#include "lib/pid.h"
#include "lib/transmitter.h"
#include "lib/motor.h"

/** Static variables -------------------------------------------------------- */
static pid_instance_t pitch_pid;
static pid_instance_t roll_pid;
static pid_instance_t yaw_pid;
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

	app_timer_t* gyro_timer = create_timer(print_flag_cb, 500, 50);

	while (!angles_is_init());

	pid_create(&pitch_pid, 3.0, 0.5, 0.1, 0.0, 0.0);
	pid_create(&roll_pid, 3.0, 0.5, 0.1, 0.0, 0.0);
	//pid_create(&yaw_pid, 1.0, 0.0, 0.0, 0.0, 0.0);

	while (true) {
		if (print_flag) {
			print_flag = false;
			angles_t angles = get_angles();

			double pitch_out = pid_fire(pitch_pid, 0, angles.pitch);
			double roll_out = pid_fire(roll_pid, 0, angles.roll);
			double yaw_out = pid_fire(yaw_pid, 0, angles.yaw);

			transmitter_read();
			transmitter tm = get_transmitter_values();

			double motor_0 = tm.throttle - pitch_out - roll_out;
			double motor_1 = tm.throttle + pitch_out - roll_out;
			double motor_2 = tm.throttle + pitch_out + roll_out;
			double motor_3 = tm.throttle - pitch_out + roll_out;

			if (tm.throttle < 1200) {
				motor_0 = 0;
				motor_1 = 0;
				motor_2 = 0;
				motor_3 = 0;
			}

			motor_run(0, (int) motor_0);
			motor_run(1, (int) motor_1);
			motor_run(2, (int) motor_2);
			motor_run(3, (int) motor_3);

			//pthread_mutex_lock(&print_mtx);
			//printf("Motor: %.2f, %.2f, %.2f\n", pitch_out, roll_out, yaw_out);
			//pthread_mutex_unlock(&print_mtx);
		}
	}
}
