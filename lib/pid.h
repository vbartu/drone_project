#ifndef __PID_H
#define __PID_H

typedef struct pid_instance_t {
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
} pid_instance_t;

void pid_create(pid_instance_t* pid, double kp, double ki, double kd, double kr,
		double kf);

double pid_fire(pid_instance_t pid, double input, double feedback);

#endif // __PID_H
