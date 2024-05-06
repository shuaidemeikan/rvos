#include "os.h"

/*
 * Following functions SHOULD be called ONLY ONE time here,
 * so just declared here ONCE and NOT included in file os.h.
 */
extern void uart_init(void);
extern void page_init(void);
extern void sched_init(void);
extern void schedule(void);
extern void os_main(void);
extern void trap_init(void);
extern void plic_init(void);
extern void timer_init(void);
extern void virtio_init();
extern void byte_test();
extern void mempage_init();
extern void stdio_init();
extern void rand_init();

uint32_t curr_pri = 11;

void test(void)
{
	byte_test();
}

void start_kernel(void)
{
	uart_init();
	uart_puts("Hello, RVOS!\n");

	rand_init();

	virtio_init();
	page_init();

	trap_init();

	plic_init();

	timer_init();

	stdio_init();

	sched_init();

	test();

	mempage_init();

	os_main();

	schedule();

	uart_puts("Would not go here!\n");
	while (1) {}; // stop here!
}

