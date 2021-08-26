#include <stdio.h>
#include <stdint.h>
#include <malloc.h>

#include "pid.h"
#include "timer.h"
#include "main.h"
#include "lib/fixedptc.h"


void pid_create(pid_instance_t* pid, fixedpt kp, fixedpt ki, fixedpt kd, fixedpt kr,
	fixedpt kf)
{
	pid->kp = kp;
	pid->ki = ki;
	pid->kd = kd;
	pid->kr = kr;
	pid->kf = kf;
	pid->ki_accum = fixedpt_rconst(0.0);
	pid->kd_last = fixedpt_rconst(0.0);
	pid->kr_accum = fixedpt_rconst(0.0);
	pid->kf_last = fixedpt_rconst(0.0);
	pid->last_time = micros() ;
}

fixedpt pid_fire(pid_instance_t pid, fixedpt input, fixedpt feedback)
{
	uint64_t current_time = micros();
	fixedpt elapsed_time = fixedpt_rconst((current_time - pid.last_time) / 1000000.0);
	pid.last_time = fixedpt_rconst(current_time);


	fixedpt error;
	if (pid.kf != 0) {
		fixedpt cf = fixedpt_mul(pid.kf, fixedpt_div((feedback + pid.kf_last), elapsed_time));
		error = input - fixedpt_mul(feedback, cf);
	} else {
		 error = input - feedback;
	}

	pid.kf_last = feedback;

	pid.ki_accum += error * elapsed_time;
	fixedpt cd = fixedpt_mul(pid.kd, fixedpt_div((error - pid.kd_last), elapsed_time));
	pid.kd_last = error;
	
	fixedpt pid_out = fixedpt_mul(pid.kp, error ) + cd;

	return pid_out;
}
