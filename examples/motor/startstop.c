
#include <stdio.h>
#include <stdint.h>
#include <machine/patmos.h>
#include <machine/spm.h>
#include "lib/transmitter.h"

#include <stdbool.h>
#include <pthread.h>
#include <pthread_mutex.h>

static int counter;
static pthread_mutex_t counter_mtx = PTHREAD_MUTEX_INITIALIZER;
transmitter tm;

//motors
volatile _SPM int* led_ptr = (volatile _SPM int*) PATMOS_IO_LED;
#define MOTOR ( ( volatile _IODEV unsigned * )  PATMOS_IO_ACT+0x10 )
volatile _SPM int* m = (volatile _SPM int*) PATMOS_IO_ACT+0x10;

static int counter;
static int i=0,j=1,k=2000;
static bool exitmotor = false, final = false;

static void actuator(void)
{
  pthread_mutex_lock(&counter_mtx);
  counter = j;
  pthread_mutex_unlock(&counter_mtx);
  if (!exitmotor)
  {
    j = j + 100;
    if (j >= 2000) j = 0;
    *(MOTOR + 0) = j;
    *(MOTOR + 1) = j;
    *(MOTOR + 2) = j;
    *(MOTOR + 3) = j;
  }
}

static void actuator_stop(void)
{
  pthread_mutex_lock(&counter_mtx);
  if (!final)
  {
    counter = k;
    exitmotor = true;
    k = k - 10;
    if (k <= 0) k = 0;
    *(MOTOR + 0) = k;
    *(MOTOR + 1) = k;
    *(MOTOR + 2) = k;
    *(MOTOR + 3) = k;
    printf("\n Stopping motor: %d", k);
  }
  if (k == 0)
  {
    final = true;
    printf("\n Stop Motor, throttle:%d", tm.throttle);
  }
  pthread_mutex_unlock(&counter_mtx);
}


static void* thread1_main(void *args)
{
  while(1);
}


static void* thread3_main(void *args)
{
  while(1);
}

static void* thread2_main(void *args)
{

  while(1){
    if (!exitmotor)
      actuator();
    if (exitmotor )
      actuator_stop();

    pthread_mutex_lock(&counter_mtx);
    tm = get_transmitter_values();
    if (tm.throttle < 1200 && tm.throttle > 800 )
      exitmotor = true;
    pthread_mutex_unlock(&counter_mtx);

    transmitter_read();  

    if (final)
      break;

    while (i > 0)
    {
      i--;
    }
    if (i == 0)
      i = 200000;
  }

}


int main() {
  pthread_t thread1, thread2, thread3;
  pthread_create(&thread1, NULL, thread1_main, NULL);
  pthread_create(&thread2, NULL, thread2_main, NULL);
  pthread_create(&thread3, NULL, thread3_main, NULL);

  while(true){
    * led_ptr = 0xff;
    pthread_mutex_lock(&counter_mtx);
    printf("\nCounter:%d", counter);
    pthread_mutex_unlock(&counter_mtx);
    if (final)
      while (i > 0)
      {
        i--;
      }
      if (i == 0)
        i = 200000;
      break;
  }
  return 0;
}
