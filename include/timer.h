#ifndef _TIMER_H
#define _TIMER_H

#include "types.h"
#include "platform.h"
#include "riscv.h"
#include <stddef.h>
#include "lock.h"
#include "printf.h"

#define TIMER_INTERVAL CLINT_TIMEBASE_FREQ
#define MAX_TIMER 10

/* software timer */
struct timer {
	void (*func)(void *arg);
	void *arg;
	uint32_t timeout_tick;
};

void timer_load(int interval);
void timer_init();
struct timer *timer_create(void (*handler)(void *arg), void *arg, uint32_t timeout);
void timer_delete(struct timer *timer);
void timer_handler() ;

#endif // !_TIMER_H

