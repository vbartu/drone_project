#include <stdio.h>
#include <stdint.h>

#include "timer.h"

int counter = 0;


static void timer_callback(void);


static void timer_callback(void)
{
	counter++;
}


int main(void)
{
	timer_init();
	app_timer_t* timer = create_timer(timer_callback, 500, 1000);
	printf("App timer address: 0x%08x\n", (int) timer);

	while (1) {
		uint32_t time = (uint32_t) (micros()/1000);
		printf("Time passed: %d, %lu\n", counter, time);
		for (int i = 0; i < 50000000; i++);
	}

	return 0;
}
