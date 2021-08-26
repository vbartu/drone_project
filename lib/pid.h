#ifndef __PID_H
#define __PID_H

#include "lib/fixedptc.h"

typedef struct pid_instance_t {
	fixedpt kp;
	fixedpt ki;
	fixedpt kd;
	fixedpt kr;
	fixedpt kf;
	fixedpt ki_accum;
	fixedpt kd_last;
	fixedpt kr_accum;
	fixedpt kf_last;
	uint64_t last_time;
} pid_instance_t;

void pid_create(pid_instance_t* pid, fixedpt kp, fixedpt ki, fixedpt kd, fixedpt kr,
	fixedpt kf);

double pid_fire(pid_instance_t pid, fixedpt input, fixedpt feedback);

#endif // __PID_H
