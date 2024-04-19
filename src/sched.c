#include "sched.h"

/* defined in entry.S */
extern void switch_to(struct context *next);

/*
 * In the standard RISC-V calling convention, the stack pointer sp
 * is always 16-byte aligned.
 */
uint8_t __attribute__((aligned(16))) task_stack[MAX_TASKS][STACK_SIZE];
struct context ctx_tasks[MAX_TASKS];

// 就绪队列
task_struct ready_task[MAX_TASKS];
// 当前进行
task_struct cur_task = {.pid = -1};

/*
 * _top is used to mark the max available position of ctx_tasks
 * _current is used to point to the context of current task
 */
static int _top = 0;
static int _current = -1;

void sched_init()
{
	w_mscratch(0);

	/* enable machine-mode software interrupts. */
	w_mie(r_mie() | MIE_MSIE);
}

/*
 * implment a simple cycle FIFO schedular
 */
void schedule()
{
	if (_top <= 0) {
		panic("Num of task should be greater than zero!");
		return;
	}

	// 判断是否需要切
	// 创建0进程时，无需判断
	if (cur_task.pid == -1)
	{
		cur_task = ready_task[0];
		goto sw;
	}
	// 判断时间片是否耗尽
	cur_task.counter--;
	if (cur_task.counter <= 0)
		goto sw;
	
	return;
	
sw:
	// 先保存当前的寄存器数据到tss中
	// save_context(&(cur_task.tss));

	_current = (_current + 1) % _top;
	cur_task = ready_task[_current];
	struct context *next = &(cur_task.tss);
	extern int curr_pri;
	curr_pri = 01;
	switch_to(next);
}

/*
 * DESCRIPTION
 * 	Create a task.
 * 	- start_routin: task routine entry
 * RETURN VALUE
 * 	0: success
 * 	-1: if error occured
 */
// int task_create(void (*start_routin)(void))
// {
// 	if (_top < MAX_TASKS) {
// 		ctx_tasks[_top].sp = (reg_t) &task_stack[_top][STACK_SIZE];
// 		ctx_tasks[_top].pc = (reg_t) start_routin;
// 		_top++;
// 		return 0;
// 	} else {
// 		return -1;
// 	}
// }

/*
 * DESCRIPTION
 * 	task_yield()  causes the calling task to relinquish the CPU and a new 
 * 	task gets to run.
 */
void task_yield()
{
	/* trigger a machine-level software interrupt */
	int id = r_mhartid();
	*(uint32_t*)CLINT_MSIP(id) = 1;
}

/*
 * a very rough implementaion, just to consume the cpu
 */
void task_delay(volatile int count)
{
	count *= 50000;
	while (count--);
}

static void progress0()
{
	while (1)
	{
		printf("hello1 progress0\n");
		task_delay(4000);
		printf("hello2 progress0\n");
		task_delay(4000);
		printf("hello3 progress0\n");
		task_delay(4000);
		printf("hello4 progress0\n");
		task_delay(4000);
		printf("hello5 progress0\n");
		task_delay(4000);
		printf("hello6 progress0\n");
		task_delay(4000);
		printf("hello7 progress0\n");
		task_delay(4000);

	}
}

static void progress1()
{
   	while (1)
	{
		printf("hello1 progress1\n");
		task_delay(4000);
		printf("hello2 progress1\n");
		task_delay(4000);
		printf("hello3 progress1\n");
		task_delay(4000);
		printf("hello4 progress1\n");
		task_delay(4000);
		printf("hello5 progress1\n");
		task_delay(4000);
		printf("hello6 progress1\n");
		task_delay(4000);
		printf("hello7 progress1\n");
		task_delay(4000);
		printf("hello8 progress1\n");
		task_delay(4000);
	}
}

// 初始化进程，目前是创建0号进程
void init_progress()
{
    task_struct* task0 = (task_struct*)byte_alloc(sizeof(task_struct*));
    task0->pid = 0;
    task0->counter = 5;
    task0->exit_code = 0;
    task0->father = -1;
    task0->priority = 0;
    task0->state = 0;
    task0->tty = 0;
    task0->tss.sp = byte_alloc(1024);
    task0->tss.pc = progress0;
	ready_task[_top] = *task0;
	_top++;


	task_struct* task01 = (task_struct*)byte_alloc(sizeof(task_struct*));
    task01->pid = 1;
    task01->counter = 5;
    task01->exit_code = 0;
    task01->father = -1;
    task01->priority = 0;
    task01->state = 0;
    task01->tty = 0;
    task01->tss.sp = byte_alloc(1024);
    task01->tss.pc = progress1;
	ready_task[_top] = *task01;
	_top++;
}

int task_create(void* start_routin, uint32_t counter, uint32_t priority, uint32_t tty)
{
	if (_top < MAX_TASKS) {
		ctx_tasks[_top].sp = byte_alloc(1024);
		ctx_tasks[_top].pc = start_routin;
		_top++;
		return 0;
	} else {
		return -1;
	}
}