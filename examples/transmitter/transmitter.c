#include <stdio.h>
#include <stdlib.h>
#include <machine/exceptions.h>
#include <stdbool.h>
#include <math.h>
#include <stdbool.h>
#include <pthread.h>
#include <pthread_mutex.h>
#include <machine/patmos.h>
#include <machine/spm.h>


static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
#define RECEIVER ( ( volatile _IODEV unsigned * ) PATMOS_IO_ACT )
volatile _SPM int* led_ptr = (volatile _SPM int*) PATMOS_IO_LED;

const unsigned int CPU_PERIOD = 20;

// (roll,pitch,yaw,throttle)
int low[4]	= { 1000, 1000, 1000, 1090 };
int center[4]	= { 1500, 1500, 1495, 1530 };
int high[4]	= { 2000, 2000, 2000, 2000 };

// 6 channels of transmitter
__int32_t channel_roll;
__int32_t channel_pitch;
__int32_t channel_throttle;
__int32_t channel_yaw;
__int32_t channel_5;
__int32_t channel_6;


int receiver_read( unsigned int receiver_id )
{
    unsigned int clock_cycles_counted 	= *( RECEIVER + receiver_id );
    unsigned int pulse_high_time 	= ( clock_cycles_counted * CPU_PERIOD ) / 1000;

    return pulse_high_time;
}

int convert_receiver_channel( unsigned int channel, bool reverse )
{
  int actual, difference;

  actual = receiver_read( channel );

  if( actual < center[channel] )
  {
    if( actual < low[channel] )
    	actual = low[channel];
    	
    difference = ( (long)(center[channel] - actual) * (long)500) / ( center[channel] - low[channel] );
    
    if ( reverse )
        return 1500 - difference;
    else
    	return 1500 + difference;
  }
  else if( actual > center[channel] )
  {
    if( actual > high[channel] )
    	actual = high[channel];

    difference = ( (long)(actual - center[channel] ) * (long)500 ) / ( high[channel] - center[channel] );
    
    if ( reverse )
    	return 1500 + difference;
    else
        return 1500 - difference;
  }
  else 
  	return 1500;
}



void transmitter_read() {
    // read the receiver pwm duty cycle

    int channel_roll_tmp 	= convert_receiver_channel(0, false);
    int channel_pitch_tmp 	= convert_receiver_channel(1, true);
    int channel_throttle_tmp 	= convert_receiver_channel(2, false);
    int channel_yaw_tmp 	= convert_receiver_channel(3, true); 
    int channel_5_tmp 		= receiver_read(4);
    int channel_6_tmp 		= receiver_read(5);
    
    pthread_mutex_lock( &mutex );    
    channel_roll 	= channel_roll_tmp;
    channel_pitch 	= channel_pitch_tmp;
    channel_throttle 	= channel_throttle_tmp;
    channel_yaw 	= channel_yaw_tmp;
    channel_5 		= channel_5_tmp;
    channel_6 		= channel_6_tmp;
    pthread_mutex_unlock( &mutex );

}

static void* thread1_main( void *args )
{
	while(1);
}

static void* thread2_main( void *args )
{
	while(1)
		transmitter_read();
}

int main() {
	pthread_t thread1, thread2, thread3;
	pthread_create( &thread1, NULL, thread1_main, NULL );
	pthread_create( &thread2, NULL, thread2_main, NULL );
	pthread_create( &thread3, NULL, thread1_main, NULL );
	while(1){
		* led_ptr = 0xff;

		pthread_mutex_lock( &mutex );
		printf( "\n Channel-roll\t:%d",channel_roll );
		printf( "\n Channel-pitch\t:%d",channel_pitch );
		printf( "\n Channel-throttle\t:%d",channel_throttle );
		printf( "\n Channel-yaw\t:%d",channel_yaw );
		printf( "\n Channel-5\t:%d",channel_5 );
		printf( "\n Channel-6\t:%d",channel_6 );
		pthread_mutex_unlock( &mutex );
		
		int i =2000000;
		while( i>0 )
			i--;
	}		
	return 0;
}
