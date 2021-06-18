#include <stddef.h>
#include <machine/exceptions.h>
#include <machine/rtc.h>
#include <pthread_mutex.h>

#include "timer.h"

#define __PATMOS_TIMER_BASE       0xF0020000
#define __PATMOS_TIMER_HICLK      (__PATMOS_TIMER_BASE + 0x00)
#define __PATMOS_TIMER_LOCLK      (__PATMOS_TIMER_BASE + 0x04)
#define __PATMOS_TIMER_HIUSEC     (__PATMOS_TIMER_BASE + 0x08)
#define __PATMOS_TIMER_LOUSEC     (__PATMOS_TIMER_BASE + 0x0c)

#define TIMER_LIST_SIZE 15

static app_timer_t timer_list[TIMER_LIST_SIZE];
static pthread_mutex_t timer_list_mtx = PTHREAD_MUTEX_INITIALIZER;

static void us_interrupt_cb(void) __attribute__((naked));
static void set_next_timer(void);
static void execute_timer(void);


static void us_interrupt_cb(void)
{
	exc_prologue();
	execute_timer();
	exc_epilogue();
}

uint64_t micros(void)
{
	return (uint64_t) get_cpu_usecs();
}

static void set_next_timer(void)
{
	int earliest_index = -1;
	int64_t earliest_time = 0;

	for (int i = 0; i < TIMER_LIST_SIZE; i++) {
		if (timer_list[i].timer_cb != NULL) {
			if (earliest_index == -1) {
				earliest_index = i;
				earliest_time = timer_list[i].time;
			} else if (earliest_time <= 0
					&& timer_list[i].time < timer_list[earliest_index].time) {
				earliest_index = i;
				earliest_time = timer_list[i].time;
			}
		}
	}
	
	if (earliest_index != -1) {
		arm_usec_timer(earliest_time);	
	}
}

static void execute_timer(void)
{
	uint64_t current_time = micros();
	for (int i = 0; i < TIMER_LIST_SIZE; i++) {
		if (timer_list[i].timer_cb != NULL) {
			if (timer_list[i].time <= current_time) {
				timer_list[i].timer_cb();
				if (timer_list[i].period) {
					timer_list[i].time += timer_list[i].period*1000;
				} else {
					timer_list[i].timer_cb = NULL;
				}
			}
		}
	}
	set_next_timer();
}


void timer_init(void)
{
	exc_register(17, us_interrupt_cb);
	intr_unmask_all();
	intr_clear_all_pending();
	intr_enable();
}

app_timer_t* create_timer(timer_cb_t timer_cb, uint32_t time_ms, uint32_t period_ms)
{
	app_timer_t* result_timer = NULL;

	pthread_mutex_lock(&timer_list_mtx);
	uint64_t current_time = micros();

	for (int i = 0; i < TIMER_LIST_SIZE; i++) {
		if (timer_list[i].timer_cb == NULL) {
			timer_list[i].id = i;
			timer_list[i].timer_cb = timer_cb;
			timer_list[i].time = current_time + time_ms*1000;
			timer_list[i].period = period_ms;
			result_timer = &(timer_list[i]);
			set_next_timer();
			break;
		}
	}
	pthread_mutex_unlock(&timer_list_mtx);

	return result_timer;
}

void delete_timer(app_timer_t* timer)
{
	pthread_mutex_lock(&timer_list_mtx);
	if (timer_list[timer->id].timer_cb != NULL) {
		timer_list[timer->id].timer_cb = NULL;
		set_next_timer();
	}
	pthread_mutex_unlock(&timer_list_mtx);
}
