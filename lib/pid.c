#include <stdio.h>
#include <stdint.h>
#include <malloc.h>

#include "pid.h"
#include "timer.h"
#include "main.h"

struct pid_instance_t {
	double Kp;
	double Ki;
	double Kd;
	double Kr;
	double Kf;
	double Ki_accum;
	double Kd_last;
	double Kr_accum;
	double Kf_last;
	uint64_t last_time;
};


void pid_create(pid_ints_t pid, double kp, double ki, double kd, double kr,
		double kf)
{
	pid->Kp = kp;
	pid->Ki = ki;
	pid->Kd = kd;
	pid->Kr = kr;
	pid->Kf = kf;
	pid->Ki_accum = 0.0;
	pid->Kd_last = 0.0;
	pid->Kr_accum = 0.0;
	pid->Kf_last = 0.0;
	pid->last_time = micros();
}

double pid_fire(pid_ints_t pid, double input, double feedback)
{
	uint64_t current_time = micros();
	double elapsed_time = (current_time - pid->last_time) / 1000000.0;
	pid->last_time = current_time;

	//double filtered_fb = pid->Kf * (pid->Kf_last - feedback) / elapsed_time;
	//pid->Kf_last = feedback;

	//double error = filtered_fb + input;

	//pid->Ki_accum += error*elapsed_time;
	//double pid_out = (pid->Kp * error
	//		        + pid->Ki * pid->Ki_accum
	//				+ pid->Kd * (pid->Kd_last - error) / elapsed_time);
	//pid->Kd_last = error;


	//pid->Kr_accum += pid_out * elapsed_time;
	//double output = pid->Kr * pid->Kr_accum;

	//return output;
	double error = input + feedback;

	double output = pid->Kp * error;

	pthread_mutex_lock(&print_mtx);
	printf("Error %.2f -- %.2f -- %.2f\n", error, output, pid->Kp);
	pthread_mutex_unlock(&print_mtx);

	return output;
}
