#include <stdint.h>
#include <malloc.h>

#include "pid.h"
#include "timer.h"

struct pid_instance_t {
	int Kp;
	int Ki;
	int Kd;
	int Kr;
	int Kf;
	double Ki_accum;
	double Kd_last;
	double Kr_accum;
	double Kf_last;
	uint64_t last_time;
};


void pid_create(pid_ints_t pid, pid_parameters_t config)
{
	struct pid_instance_t instance = {
		config.k_proportional,
		config.k_integral,
		config.k_derivative,
		config.k_response,
		config.k_filter,
		0.0,
		(double) micros(),
		0.0,
		(double) micros(),
		micros()
	};
	pid = (pid_ints_t) &instance;
}

double pid_fire(pid_ints_t pid, double input, double feedback)
{
	uint64_t current_time = micros();
	uint64_t elapsed_time = (current_time - pid->last_time) / 1000000.0;
	pid->last_time = current_time;

	double filtered_fb = pid->Kf * (pid->Kf_last - feedback) / elapsed_time;
	pid->Kf_last = feedback;

	double error = filtered_fb + input;

	pid->Ki_accum += error*elapsed_time;
	double pid_out = (pid->Kp * error
			        + pid->Ki * pid->Ki_accum
					+ pid->Kd * (pid->Kd_last - error) / elapsed_time);
	pid->Kd_last = error;


	pid->Kr_accum += pid_out * elapsed_time;
	double output = pid->Kr * pid->Kr_accum;

	return output;
}
