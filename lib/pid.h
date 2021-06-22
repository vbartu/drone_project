#ifndef __PID_H
#define __PID_H

typedef struct pid_instance_t* pid_ints_t;

typedef struct pid_parameters_t {
	int k_proportional;
	int k_integral;
	int k_derivative;
	int k_response;
	int k_filter;
} pid_parameters_t;


void pid_create(pid_ints_t pid, pid_parameters_t config);

double pid_fire(pid_ints_t pid, double input, double feedback);

#endif // __PID_H
