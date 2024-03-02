#ifndef __OS_H__
#define __OS_H__

#include "types.h"
#include "riscv.h"
#include "platform.h"

#include <stddef.h>
#include <stdarg.h>
#include <string.h>

/* uart */
extern int uart_putc(char ch);
extern void uart_puts(char *s);
extern int uart_getc(void);

/* printf */
extern int  printf(const char* s, ...);
extern void panic(char *s);

/*memtools*/
extern void* memcpy(void* ptr1, const void* ptr2, size_t len);
extern int memcmp(const void *ptr1, const void *ptr2, size_t num);
extern void* memset(void *s, int c, size_t n);

/* memory management */
extern void *page_alloc(int npages);
extern void page_free(void *p);


extern int  task_create(void (*task)(void));
extern void task_delay(volatile int count);
extern void task_yield();

/* plic */
extern int plic_claim(void);
extern void plic_complete(int irq);

/* lock */
extern int spin_lock(void);
extern int spin_unlock(void);


extern struct timer *timer_create(void (*handler)(void *arg), void *arg, uint32_t timeout);
extern void timer_delete(struct timer *timer);

#endif /* __OS_H__ */
