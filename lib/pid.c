#include <stdio.h>
#include <stdint.h>
#include <malloc.h>

#include "pid.h"
#include "timer.h"
#include "main.h"


void pid_create(pid_ints_t pid, double kp, double ki, double kd, double kr,
		double kf)
{
	pid.Kp = kp;
	pid.Ki = ki;
	pid.Kd = kd;
	pid.Kr = kr;
	pid.Kf = kf;
	pid.Ki_accum = 0.0;
	pid.Kd_last = 0.0;
	pid.Kr_accum = 0.0;
	pid.Kf_last = 0.0;
	pid.last_time = micros();
}

double pid_fire(pid_ints_t pid, double input, double feedback)
{
	uint64_t current_time = micros();
	double elapsed_time = (current_time - pid.last_time) / 1000000.0;
	pid.last_time = current_time;


	double Cf = pid.Kd * (feedback - pid.Kf_last) / elapsed_time);
	double error = input-(feedback*Cf);

	double Ci = pid.Ki * pid.Ki_accum;
	double Cd = pid.Kd * (error - pid.Kd_last) / elapsed_time);
	double C = (pid.Kp * error + Ci + Cd);

	double pid_out = (pid.Kp * error + Ci + Cd);

	pid.Ki_accum += error * elapsed_time;
	pid.Kd_last = error;
	pid.Kf_last = feedback;

	return pid_out;
}
