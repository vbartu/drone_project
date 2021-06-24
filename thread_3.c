#include <stdio.h>

#include "main.h"
#include "lib/uart.h"
#include "lib/timer.h"

static uint8_t buffer[512];

void* thread_3_main(void* args)
{
	pthread_mutex_lock(&print_mtx);
	printf("Thread 3\n");
	pthread_mutex_unlock(&print_mtx);

	while (1) {
		if (uart_data_ready()) {
			uint8_t c = uart_read();

			pthread_mutex_lock(&print_mtx);
			printf("Data readed: %c, %d\n", (char) c, (int) c);
			pthread_mutex_unlock(&print_mtx);
		}
		pthread_mutex_lock(&print_mtx);
		printf("hola\n");
		pthread_mutex_unlock(&print_mtx);
		delay(1000);
	}

}
