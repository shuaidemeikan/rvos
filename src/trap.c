#include "trap.h"

extern void trap_vector(void);
extern void uart_isr(void);
extern void timer_handler(void);
extern void schedule(void);

void trap_init()
{
	/*
	 * set the trap-vector base-address for machine-mode
	 */
	w_mtvec((reg_t)trap_vector);
}

void external_interrupt_handler()
{
	int irq = plic_claim();

	if (irq == UART0_IRQ){
      		uart_isr();
	} else if (irq) {
		printf("unexpected interrupt irq = %d\n", irq);
	}
	
	if (irq) {
		plic_complete(irq);
	}
}

reg_t trap_handler(reg_t epc, reg_t cause, struct context *cxt)
{
	// 进入中断/异常处理程序，需要先保存上下文
	// 因为可能是会导致切换的中断，如果发生切换，那么就需要发生中断前上下文了
	// extern task_struct cur_task;
	// save_context(&(cur_task.tss));
	// cur_task.tss.pc = r_mepc;

	extern int curr_pri;
	curr_pri = 11;
	reg_t return_pc = epc;
	reg_t cause_code = cause & 0xfff;
	
	if (cause & 0x80000000) {
		/* Asynchronous trap - interrupt */
		switch (cause_code) {
		case 3:
			uart_puts("software interruption!\n");
			/*
			 * acknowledge the software interrupt by clearing
    			 * the MSIP bit in mip.
			 */
			int id = r_mhartid();
    			*(uint32_t*)CLINT_MSIP(id) = 0;

			schedule();

			break;
		case 7:
			//uart_puts("timer interruption!\n");
			timer_handler();
			break;
		case 11:
		//	uart_puts("external interruption!\n");
			external_interrupt_handler();
			break;
		default:
			uart_puts("unknown async exception!\n");
			break;
		}
	} else {
		/* Synchronous trap - exception */
		//printf("Sync exceptions!, code = %d\n", cause_code);
		switch (cause_code) {
		case 8:
			//uart_puts("System call from U-mode!\n");
			do_syscall(cxt);
			return_pc += 4;
			break;
		case 9:
			//uart_puts("System call from S-mode!\n");
			do_syscall(cxt);
			return_pc += 4;
			break;
		case 2:
			uart_puts("Illegal instruction\n");
			printf("virtual addr: %d\n", r_stval());
			panic("OOPS! What can I do!");
			break;
		// 由于页表采用了写时复制的方式，所以如果出现下面三种异常，那八成是页表写时复制的原因
		// 需要新建一个二级页表的映射到这里，随后更改一级页表的权限
		case 12:
			uart_puts("Instruction page fault\n");
			printf("virtual addr: %d\n", r_stval());
			printf("sepc = : %d\n", r_sepc());
		case 13:
			uart_puts("Load access fault\n");
			printf("virtual addr: %d\n", r_stval());
			printf("sepc = : %d\n", r_sepc());
		case 15:
			uart_puts("Store/AMO access fault\n");
			printf("virtual addr: %x, %x\n", r_mtval(), r_mepc());
			//panic("OOPS! What can I do!");
			extern task_struct cur_task;
			alloc_secpage(cur_task.page, r_mtval());

			break;
		default:
			uart_puts("unknow exception\n");
			panic("OOPS! What can I do!");
			//return_pc += 4;
		}
	}
	curr_pri = 01;
	return return_pc;
}

void trap_test()
{
	/*
	 * Synchronous exception code = 7
	 * Store/AMO access fault
	 */
	*(int *)0x00000000 = 100;

	/*
	 * Synchronous exception code = 5
	 * Load access fault
	 */
	//int a = *(int *)0x00000000;

	uart_puts("Yeah! I'm return back from trap!\n");
}

