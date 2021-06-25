#ifndef __PID_H
#define __PID_H

typedef struct pid_instance_t* pid_ints_t;

void pid_create(pid_ints_t pid, double kp, double ki, double kd, double kr,
		double kf);

double pid_fire(pid_ints_t pid, double input, double feedback);

#endif // __PID_H
