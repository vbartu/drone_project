#include <stdio.h>
#include <stdbool.h>

#include "main.h"
#include "lib/angles.h"
#include "lib/timer.h"

/** Static variables -------------------------------------------------------- */
static volatile bool read_angles_flag;
static int inner_c, outer_c;
static volatile bool print_flag;

/** Function prototypes ----------------------------------------------------- */
static void read_angles_flag_cb(void);

/** Function definitions ---------------------------------------------------- */
static void read_angles_flag_cb(void)
{
	read_angles_flag = true;
}

static void print_flag_cb(void)
{
	print_flag = true;
}

/** Public functions -------------------------------------------------------- */
void* thread_1_main(void* args)
{
	pthread_mutex_lock(&print_mtx);
	printf("Thread 1\n");
	pthread_mutex_unlock(&print_mtx);

	angles_init();
	timer_init();
	app_timer_t* angles_timer = create_timer(read_angles_flag_cb, 500, 75);
	app_timer_t* gyro_timer = create_timer(print_flag_cb, 500, 10000);

	while (true) {
		if (read_angles_flag) {
			read_angles_flag = false;
			//uint64_t t1 = micros();
			calculate_angles();
			//uint64_t t2 = micros();
			inner_c++;

		}
		outer_c++;

		if (print_flag) {
			print_flag = false;
			pthread_mutex_lock(&print_mtx);
			printf("Time: %d, %d\n", inner_c, outer_c);
			pthread_mutex_unlock(&print_mtx);
		}
	}
}
