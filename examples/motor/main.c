
#include <stdio.h>
#include <stdint.h>
#include <machine/patmos.h>
#include <machine/spm.h>

#include <stdbool.h>
#include <pthread.h>
#include <pthread_mutex.h>

static int counter;
static pthread_mutex_t counter_mtx = PTHREAD_MUTEX_INITIALIZER;
static int sec_counter;
static bool print_flag;


//motors
volatile _SPM int* led_ptr = (volatile _SPM int*) PATMOS_IO_LED;
#define MOTOR ( ( volatile _IODEV unsigned * )  PATMOS_IO_ACT+0x10 )
volatile _SPM int* m = (volatile _SPM int*) PATMOS_IO_ACT+0x10;
#define m1 0
#define m2 1
#define m3 2
#define m4 3

static int counter;
static int i;

static void actuator(void)
{


		* MOTOR = 1000;
		*(MOTOR + 1) = 1000;
		*(MOTOR + 2) = 1000;
		*(MOTOR + 3) = 1000;
	i = 2000000;
	while(i>0)
		i--;
/*
	*n = 1000;
	*(m+1) = 1000;	
	*(m+2) = 1000;
	*(m+3) = 1000;*/
}


static void* thread1_main(void *args)
{
	while(1);
}


static void* thread2_main(void *args)
{
	while(1);
}

static void* thread3_main(void *args)
{
	while(1){
	//	actuator();
		pthread_mutex_lock(&counter_mtx);
		counter++;
		pthread_mutex_unlock(&counter_mtx);

	}
}

int main() {
	pthread_t thread1, thread2, thread3;
	pthread_create(&thread1, NULL, thread3_main, NULL);
	pthread_create(&thread2, NULL, thread2_main, NULL);
	pthread_create(&thread3, NULL, thread1_main, NULL);
//	actuator();
	while(1){
		* led_ptr = 0xff;

		pthread_mutex_lock(&counter_mtx);
		printf("\nCounter:%d",counter);
		pthread_mutex_unlock(&counter_mtx);
	}		
	return 0;
}
