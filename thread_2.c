#include <stdio.h>
#include <stdbool.h>

#include "main.h"
#include "lib/angles.h"
#include "lib/timer.h"
#include "lib/pid.h"
#include "lib/transmitter.h"
#include "lib/motor.h"

#define CHANGE 0.2
#define BASE_THROTTLE 1750
#define THROTTLE_SCALE 10

/** Data structures --------------------------------------------------------- */
typedef struct pid_out_t {
	double pitch;
	double roll;
	double yaw;
} pid_out_t;

/** Static variables -------------------------------------------------------- */
static pid_instance_t pitch_pid;
static pid_instance_t roll_pid;
static pid_instance_t yaw_pid;

static pid_instance_t pitch_vel_pid;
static pid_instance_t roll_vel_pid;
static pid_instance_t yaw_vel_pid;
static volatile bool controller_flag;

static pid_out_t pid_angle;
static pid_out_t pid_vel;

static volatile bool p_up_flag, p_down_flag;
static volatile bool d_up_flag, d_down_flag;

/** Function prototypes ----------------------------------------------------- */
static void controller_flag_cb(void);

/** Function definitions ---------------------------------------------------- */
static void controller_flag_cb(void)
{
	controller_flag = true;
}

/** Public functions -------------------------------------------------------- */
void* thread_2_main(void* args)
{
	pthread_mutex_lock(&print_mtx);
	printf("Thread 2\n");
	pthread_mutex_unlock(&print_mtx);

	timer_init();

	app_timer_t* controller_timer = create_timer(controller_flag_cb, 500, 50);

	while (!angles_is_init());

	pid_create(&pitch_pid, 0.4, 0.0, 0.6, 0.0, 0.0);
	pid_create(&roll_pid, 0.4, 0.0, 0.6, 0.0, 0.0);
	pid_create(&yaw_pid, 0, 0.0, 0.0, 0.0, 0.0);

	pid_create(&pitch_vel_pid, 0, 0.0, 0.0, 0.0, 0.0);
	pid_create(&roll_vel_pid, 0, 0.0, 0.0, 0.0, 0.0);
	pid_create(&yaw_vel_pid, 0.0, 0.0, 0.0, 0.0, 0.0);

	while (true) {
		if (controller_flag) {
			controller_flag = false;
			double motor_0=0, motor_1 = 0, motor_2 = 0, motor_3 = 0;
			angles_t angles_vel = get_angle_vel();
			angles_t angles = get_angles();

			pid_angle.pitch = pid_fire(pitch_pid, 0, angles.pitch);
			pid_angle.roll = pid_fire(roll_pid, 0, angles.roll);
			pid_angle.yaw = pid_fire(yaw_pid, 0, angles.yaw);

			pid_vel.pitch = pid_fire(pitch_vel_pid, pid_angle.pitch, angles_vel.pitch);
			pid_vel.roll = pid_fire(roll_vel_pid, pid_angle.roll, angles_vel.roll);
			pid_vel.yaw = pid_fire(yaw_vel_pid, pid_angle.yaw, angles_vel.yaw);

			transmitter_read();
			transmitter tm = get_transmitter_values();

			if (tm.throttle > 1800) {
				motor_0 = BASE_THROTTLE - pid_vel.pitch - pid_vel.roll - pid_vel.yaw;
				motor_1 = BASE_THROTTLE + pid_vel.pitch - pid_vel.roll + pid_vel.yaw;
				motor_2 = BASE_THROTTLE + pid_vel.pitch + pid_vel.roll - pid_vel.yaw;
				motor_3 = BASE_THROTTLE + pid_vel.roll - pid_vel.pitch + pid_vel.yaw;
			}

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
			//printf("Motor: %.3f, %.3f, %.3f, %.3f\n", motor_0, motor_1, motor_2,
					//motor_3);
			//pthread_mutex_unlock(&print_mtx);
			//
			//
			//printf("Kp: %.2f Ki: %.2f Kd: %.2f\n", pitch_pid.kp, pitch_pid.ki, pitch_pid.kd);

			printf("pitch_a: %.2f pitch_pid2: %.2f pitch_v: %.2f pitch_pid2: %.2f m1: %.2f m2: %.2f\n", angles.pitch, pid_angle.pitch, angles_vel.pitch, pid_vel.pitch, motor_0, motor_1);

			if (tm.pitch > 1900) p_up_flag = true;
			else if (tm.pitch < 1100) p_down_flag = true;
			else if (tm.pitch > 1300 && tm.pitch < 1700) {
				if (p_up_flag) {
					//pitch_pid.kp += CHANGE;
					//roll_pid.kd += CHANGE;
					//roll_pid.kd += CHANGE;
					yaw_pid.kp += 0.5;
					p_up_flag = false;
				} else if (p_down_flag) {
					//pitch_pid.kp -= CHANGE;
					//roll_pid.kd -= CHANGE;
					yaw_pid.kp -= 0.5;
					p_down_flag = false;
				}
			}

			if (tm.roll > 1900) d_up_flag = true;
			else if (tm.roll < 1100) d_down_flag = true;
			else if (tm.roll > 1300 && tm.roll < 1700) {
				if (d_up_flag) {
					//pitch_pid.kd += CHANGE;
					//roll_pid.kd += CHANGE;
					yaw_pid.kd += 0.5;
					d_up_flag = false;
				} else if (d_down_flag) {
					//pitch_pid.kd -= CHANGE;
					//roll_pid.kd -= CHANGE;
					yaw_pid.kd -= 0.5;
					d_down_flag = false;
				}
			}
		}


	}
}
