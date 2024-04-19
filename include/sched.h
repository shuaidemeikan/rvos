#ifndef _SCHED_H
#define _SCHED_H

#include "riscv.h"
#include "printf.h"
#include "platform.h"
#include "page.h"
#include "virtmem.h"

#define MAX_TASKS 10
#define STACK_SIZE 1024

/* task management */
struct context {
	/* ignore x0 */
	reg_t ra;
	reg_t sp;
	reg_t gp;
	reg_t tp;
	reg_t t0;
	reg_t t1;
	reg_t t2;
	reg_t s0;
	reg_t s1;
	reg_t a0;
	reg_t a1;
	reg_t a2;
	reg_t a3;
	reg_t a4;
	reg_t a5;
	reg_t a6;
	reg_t a7;
	reg_t s2;
	reg_t s3;
	reg_t s4;
	reg_t s5;
	reg_t s6;
	reg_t s7;
	reg_t s8;
	reg_t s9;
	reg_t s10;
	reg_t s11;
	reg_t t3;
	reg_t t4;
	reg_t t5;
	reg_t t6;
	// upon is trap frame

	// save the pc to run in next schedule cycle
	reg_t pc; // offset: 31 *4 = 124
};

typedef struct _task_struct
{
	struct context tss;     // 进程的寄存器信息
    uint32_t state;          // 0就绪，1正在运行，2阻塞
    uint32_t counter;       // 时间片计数
    uint32_t priority;      // 优先级
    uint32_t exit_code;     // 进程执行停止的退出码，父进程会取
    uint32_t pid;           // 进程号
    uint32_t father;        // 父进程号
    uint32_t tty;           // 可能有用，进程使用的tty设备号   
	mempage* page;			// 进程的页表
    // 还应该有文件系统的东西，ss暂时没做
}task_struct;

static inline void save_context(struct context *ctx) {
    asm volatile (
        "sw ra, 0(%0) \n"
        "sw sp, 4(%0) \n"
        "sw gp, 8(%0) \n"
        "sw tp, 12(%0) \n"
        "sw t0, 16(%0) \n"
        "sw t1, 20(%0) \n"
        "sw t2, 24(%0) \n"
        "sw s0, 28(%0) \n"
        "sw s1, 32(%0) \n"
        "sw a0, 36(%0) \n"
        "sw a1, 40(%0) \n"
        "sw a2, 44(%0) \n"
        "sw a3, 48(%0) \n"
        "sw a4, 52(%0) \n"
        "sw a5, 56(%0) \n"
        "sw a6, 60(%0) \n"
        "sw a7, 64(%0) \n"
        "sw s2, 68(%0) \n"
        "sw s3, 72(%0) \n"
        "sw s4, 76(%0) \n"
        "sw s5, 80(%0) \n"
        "sw s6, 84(%0) \n"
        "sw s7, 88(%0) \n"
        "sw s8, 92(%0) \n"
        "sw s9, 96(%0) \n"
        "sw s10, 100(%0) \n"
        "sw s11, 104(%0) \n"
        "sw t3, 108(%0) \n"
        "sw t4, 112(%0) \n"
        "sw t5, 116(%0) \n"
        "sw t6, 120(%0) \n"
        "csrr t0, mepc \n"
        "sw t0, 124(%0) \n"
        :
        : "r"(ctx)
        : "memory"
    );
}

void sched_init();
void schedule();
// int task_create(void (*start_routin)(void));
void task_yield();
void task_delay(volatile int count);
void init_progress();
#endif // !_SCHED_H