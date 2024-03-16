#ifndef _SCHED_H
#define _SCHED_H

#include "riscv.h"
#include "printf.h"
#include "platform.h"
#include "page.h"

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
    uint32_t state;          // 0就绪，1正在运行，2阻塞
    uint32_t counter;       // 时间片计数
    uint32_t priority;      // 优先级
    uint32_t exit_code;     // 进程执行停止的退出码，父进程会取
    uint32_t pid;           // 进程号
    uint32_t father;        // 父进程号
    uint32_t tty;           // 可能有用，进程使用的tty设备号
    struct context tss;     // 进程的寄存器信息
    // 还应该有文件系统的东西，ss暂时没做
}task_struct;

void sched_init();
void schedule();
int task_create(void (*start_routin)(void));
void task_yield();
void task_delay(volatile int count);
void init_progress();
#endif // !_SCHED_H