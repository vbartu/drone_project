#include <stdio.h>
#include <stdbool.h>

#include "main.h"
#include "lib/angles.h"

#define LOOP_PERIOD 5000 // us

/** Public functions -------------------------------------------------------- */
void* thread_1_main(void* args)
{
	pthread_mutex_lock(&print_mtx);
	printf("Thread 1\n");
	pthread_mutex_unlock(&print_mtx);

	angles_init();

	uint64_t next_time = micros();
	while (true) {
		if (next_time < micros()) {
			calculate_angles();
			next_time += LOOP_PERIOD;
		}
	}
}
