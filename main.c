#include <stdio.h>
#include <pthread.h>
#include <machine/rtc.h>

#include "main.h"

/** Static variables -------------------------------------------------------- */
pthread_mutex_t print_mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t pid_mtx = PTHREAD_MUTEX_INITIALIZER;

pid_instance_t pitch_pid;
pid_instance_t roll_pid;
pid_instance_t yaw_pid;
pid_instance_t pitch_vel_pid;
pid_instance_t roll_vel_pid;
pid_instance_t yaw_vel_pid;


/** Public functions -------------------------------------------------------- */
uint64_t micros(void)
{
	return (uint64_t) get_cpu_usecs();
}

/** Main function ----------------------------------------------------------- */
int main(void)
{
	pthread_mutex_lock(&print_mtx);
	printf("Drone controller\n");
	pthread_mutex_unlock(&print_mtx);

	pthread_t thread1, thread2, thread3;
	pthread_create(&thread1, NULL, thread_1_main, NULL);
	pthread_create(&thread2, NULL, thread_2_main, NULL);
	pthread_create(&thread3, NULL, thread_3_main, NULL);
	
	thread_0_main(NULL);

	pthread_join(thread1, NULL);
	pthread_join(thread2, NULL);
	pthread_join(thread3, NULL);
	return 0;
}
