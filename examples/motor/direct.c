
#include <stdio.h>
#include <stdint.h>
#include <machine/patmos.h>
#include <machine/spm.h>
#include "lib/transmitter.h"

#include <stdbool.h>
#include <pthread.h>
#include <pthread_mutex.h>

static int counter = 1;
static pthread_mutex_t counter_mtx = PTHREAD_MUTEX_INITIALIZER;
transmitter tm;

//motors
volatile _SPM int* led_ptr = (volatile _SPM int*) PATMOS_IO_LED;
volatile _SPM int* m = (volatile _SPM int*) PATMOS_IO_ACT+0x10;
#define MOTOR ( ( volatile _IODEV unsigned * )  PATMOS_IO_ACT+0x10 )
#define m1 0
#define m2 1
#define m3 2
#define m4 3

static int counter;
static int i=0,k=0,j=1, x=0;
static bool startmotor = false, stopmotor = false, calibrated = false;



static void motor_start(int throttle)
{
	throttle = throttle * 2 - 2000;
	if (throttle < 500) throttle = 500;
	pthread_mutex_lock(&counter_mtx);
	printf("\nMotor is starting %d", throttle);  
	pthread_mutex_unlock(&counter_mtx);
	
	
	*(MOTOR + m1) = throttle;
	*(MOTOR + m2) = throttle;
	*(MOTOR + m3) = throttle;
	*(MOTOR + m4) = throttle;
	
}



static void* thread1_main(void *args)
{
  while(1);
}



static void* thread2_main(void *args)
{	
	
	while(1){
	
		transmitter_read(); 		
		tm = get_transmitter_values();		//Reading values from the remote controller
		
		pthread_mutex_lock(&counter_mtx);
		printf("\n Throttle = %d", tm.throttle);
		pthread_mutex_unlock(&counter_mtx); 
		
		motor_start(tm.throttle);
	}
}


int main() {

	pthread_t thread1, thread2;
	pthread_create(&thread1, NULL, thread1_main, NULL);
	pthread_create(&thread2, NULL, thread2_main, NULL);

	while(true){
		*led_ptr = 0xff;
		pthread_mutex_lock(&counter_mtx);
		printf("\nCounter:%d", counter);
		pthread_mutex_unlock(&counter_mtx);
		
	}
	return 0;
}
