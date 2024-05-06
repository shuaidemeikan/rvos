#include "sched.h"
#include "platform.h"

/* defined in entry.S */
extern void switch_to(struct context *next);

// 0号进程的页表，后续所有进程的页表都来自于该页表的写时复制
extern mempage *task0_firstpage;

// shell进程的进程号
uint32_t shell_pid;

/*
 * In the standard RISC-V calling convention, the stack pointer sp
 * is always 16-byte aligned.
 */
uint8_t __attribute__((aligned(16))) task_stack[MAX_TASKS][STACK_SIZE];
struct context ctx_tasks[MAX_TASKS];

// 就绪队列
//task_struct ready_task[MAX_TASKS];
task_linkedlist* ready_task;
// 当前进行
task_struct cur_task = {.pid = -1};
// 是否由进程执行完毕而触发的sched
uint8_t is_end = 0;
// 全局pid号
uint32_t gpid = 0;
/*
 * _top is used to mark the max available position of ctx_tasks
 * _current is used to point to the context of current task
 */
static int _top = 0;
static int _current = -1;

void sched_init()
{
	w_mscratch(0);

	ready_task = initLinkedList();

	/* enable machine-mode software interrupts. */
	w_mie(r_mie() | MIE_MSIE);
}



/*
 * implment a simple cycle FIFO schedular
 */
void schedule()
{
    // 每次发生调度时，我们先重新生成一个随机数
    // 这和调度程序无关，只是为了确保每次获取的随机数足够随机
    rand_gen();

	if (_top <= 0) {
		panic("Num of task should be greater than zero!");
		return;
	}

	// 判断是否需要切
	// 创建0进程时，无需判断
	if (cur_task.pid == -1)
	{
		//cur_task = removeHead(ready_task);
		_top--;
		goto sw;
	}
	// 判断时间片是否耗尽
	cur_task.counter--;
	if (cur_task.counter <= 0)
		goto sw;
	
	// 还有一种可能会触发sched，即当有进程执行结束返回了
	// 该类情况有个特点，就是在返回时虽然在ready_task里删除了自己，但是_top的数量没剪
	// 所以只要判断_top是否比ready_task.count多1即可
	if (is_end == 1)
	{
		goto sw;
	}

    // 当进程调用了延迟类函数时，会将进程的状态阻塞，然后等待延迟结束，此时当然也要切
    if (cur_task.state == 1)
        goto sw;

	return;
	
sw:
    // 首先遍历进程，将延迟结束的进程状态改为就绪
    task_listnode *temp = ready_task->head;
    extern uint32_t last_sched_time;
    while (temp) {
        if (temp->task.state == 1)
        // 当前进程处于因延迟函数造成的阻塞态，减去上次调度到现在的时间
        // 如果等待时间已经超过了延迟时间，就将进程状态改为就绪态
        {
            temp->task.delay_time -= last_sched_time;
            last_sched_time = 0;
            if (temp->task.delay_time <= 0) {
                temp->task.state = 0;
                temp->task.delay_time = -1;
            }
        }
        temp = temp->next;
    }


	// 保存当前上下文
	if (cur_task.pid != -1)
		addTail(ready_task, cur_task);
	
	// 对因程序结束而触发sched的特殊处理
	if (is_end == 1)
	{
		removeTail(ready_task);
		is_end = 0;
	}

    // 对shell调用了quit后出发的sched的特殊处理
    extern uint8_t other_end;
    if (other_end == 1)
    {
        for (int i = 0; i < 10; i++)
        {
            // 移除所有除了shell的进程
            task_struct tmp = removeHead(ready_task);
            if (tmp.pid == shell_pid)
            {
                addTail(ready_task, cur_task);
            }
        }
        other_end = 0;
    }

	// 从队列首部移出一个上下文用以切换
	cur_task = removeHead(ready_task);
    // 不调度处于因延迟函数而阻塞的进程
    while (cur_task.state == 1)
    {
        addTail(ready_task, cur_task);
        cur_task = removeHead(ready_task);
    }

	cur_task.counter = 5;
	struct context *next = &(cur_task.tss);
	extern int curr_pri;
	curr_pri = 01;
    if (cur_task.pid != 0)
    {
        reg_t tmp = 0;
        tmp = tmp | 1U << 31 | ((((uint32_t)cur_task.page - 2048) / 4096) & 0x3FFFFF);
        w_satp(tmp);
    }
	switch_to(next);
}


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
	count *= 16666;
	while (count--);
}

static void task_loader(task_struct* task, int argc, char **argv)
{
	// 调用被加载的函数
	int retvalue = ((int (*)(int, char **))(task->start_pc))(argc, argv);

	// 进程执行完后，先删除在ready队列中删除进程
	//removeByPid(ready_task, task->pid);
	is_end = 1;
	// 再返回到sched函数，进行进程切换
    // 需要使用systemcall来调用sched，因为sched是一个内核态函数
    extern void sysendsched();
    sysendsched();

	// asm volatile("mv x1, %0"::"r"(schedule));
	// asm volatile("ret"::);
}

static int Genesis(int argc, char **argv)
{
    while(1)
    {
        task_delay(4000);
    }
}

static int progress0(int argc, char **argv)
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
	return 1;
}

int progress1(int argc, char **argv)
{
    printf("size: %d", sizeof(struct context));
    int* a = 0x87000000; 
    *a = 1;
   	//while (1)
	{
		printf("hello1 progress1\n");
		task_delay(4000);
        //usleep(10000);
		printf("hello2 progress1\n");
		task_delay(4000);
        //usleep(10000);
        printf("hello3 progress1\n");
		task_delay(4000);
        //usleep(10000);
		printf("hello4 progress1\n");
		task_delay(4000);
        //usleep(10000);
	}
    //page_free(a);
	return 1;
}

int task_create(void* start_routin, uint32_t counter, uint32_t priority, uint32_t tty, uint32_t father, int argc, char **argv)
{
	task_struct* task0 = (task_struct*)page_alloc(sizeof(task_struct*));
    task0->pid = gpid++;
    task0->counter = counter;
    task0->exit_code = 0;
    task0->father = father;
    task0->priority = priority;
    task0->state = 0;
    task0->tty = tty;
    task0->delay_time = -1;
	task0->start_pc = start_routin;
    task0->tss.sp = page_alloc(1024);
    task0->tss.pc = task_loader;
    // a0会被switch写到实际的寄存器中，随后pc跳转到task_loader，此时会将a0作为第一个参数
	task0->tss.a0 = task0;
    task0->tss.a1 = argc;
    task0->tss.a2 = argv;
    if(task0->pid != 0)
        task0->page = fork_mempage(task0_firstpage, task_loader);
    else
        task0->page = task0_firstpage;
	addTail(ready_task, *task0);
	_top++;
}

// 初始化进程，目前是创建0号进程
void init_progress()
{
    // task_struct* task0 = (task_struct*)page_alloc(sizeof(task_struct*));
    // task0->pid = gpid++;
    // task0->counter = 5;
    // task0->exit_code = 0;
    // task0->father = -1;
    // task0->priority = 0;
    // task0->state = 0;
    // task0->tty = 0;
	// task0->start_pc = progress0;
    // task0->tss.sp = page_alloc(1024);
    // task0->tss.pc = task_loader;
    // // a0会被switch写到实际的寄存器中，随后pc跳转到task_loader，此时会将a0作为第一个参数
	// task0->tss.a0 = task0;
	// addTail(ready_task, *task0);
	// _top++;
	extern int shell_loop(int argc, char **argv);
    extern int shell_test(int argc, char **argv);
    task_create(Genesis, 5, 0, 0, -1, 0, NULL);
    task_create(shell_loop, 5, 0, 0, -1, 0, NULL);
    shell_pid = 1;


	// task_struct* task01 = (task_struct*)page_alloc(sizeof(task_struct*));
    // task01->pid = gpid++;
    // task01->counter = 5;
    // task01->exit_code = 0;
    // task01->father = -1;
    // task01->priority = 0;
    // task01->state = 0;
    // task01->tty = 0;
	// task01->start_pc = progress1;
    // task01->tss.sp = page_alloc(1024);
    // task01->tss.pc = task_loader;
	// task01->tss.a0 = task01;
	// addTail(ready_task, *task01);
	// _top++;
    task_create(progress1, 5, 0, 0, -1, 0, NULL);
    //task_create(progress0, 5, 0, 0, -1, 0, NULL);
}



task_linkedlist *initLinkedList() {
    task_linkedlist *list = (task_linkedlist *)page_alloc(sizeof(task_linkedlist));
    if (list) {
        list->head = NULL;
        list->tail = NULL;
    }
	list->count = 0;
    return list;
}

void addHead(task_linkedlist *list, task_struct task) {
    task_listnode *node = (task_listnode *)page_alloc(sizeof(task_listnode));
    if (node) {
        node->task = task;
        node->prev = NULL;
        node->next = list->head;
        if (list->head) {
            list->head->prev = node;
        } else {
            list->tail = node;
        }
        list->head = node;
		list->count++;
    }
}

void addTail(task_linkedlist *list, task_struct task) {
    task_listnode *node = (task_listnode *)page_alloc(sizeof(task_listnode));
    if (node) {
        node->task = task;
        node->next = NULL;
        node->prev = list->tail;
        if (list->tail) {
            list->tail->next = node;
        } else {
            list->head = node;
        }
        list->tail = node;
		list->count++;
    }
}

void addAtIndex(task_linkedlist *list, task_struct task, int index) {
    if (index == 0) {
        addHead(list, task);
        return;
    }
    task_listnode *temp = list->head;
    for (int i = 0; temp != NULL && i < index - 1; i++) {
        temp = temp->next;
    }
    if (temp == NULL || temp->next == NULL && index > 0) {
        addTail(list, task);
    } else {
        task_listnode *node = (task_listnode *)page_alloc(sizeof(task_listnode));
        if (node) {
            node->task = task;
            node->prev = temp;
            node->next = temp->next;
            temp->next->prev = node;
            temp->next = node;
			list->count++;
        }
    }
}

// void removeHead(task_linkedlist *list) {
//     if (list->head) {
//         task_listnode *temp = list->head;
//         list->head = list->head->next;
//         if (list->head) {
//             list->head->prev = NULL;
//         } else {
//             list->tail = NULL;
//         }
//         page_free(temp);
// 		list->count--;
//     }
// }

// void removeTail(task_linkedlist *list) {
//     if (list->tail) {
//         task_listnode *temp = list->tail;
//         list->tail = list->tail->prev;
//         if (list->tail) {
//             list->tail->next = NULL;
//         } else {
//             list->head = NULL;
//         }
//         page_free(temp);
// 		list->count--;
//     }
// }

// void removeByPid(task_linkedlist *list, uint32_t pid) {
//     task_listnode *temp = list->head;
//     while (temp) {
//         if (temp->task.pid == pid) {
//             if (temp->prev) {
//                 temp->prev->next = temp->next;
//             } else {
//                 list->head = temp->next;
//             }
//             if (temp->next) {
//                 temp->next->prev = temp->prev;
//             } else {
//                 list->tail = temp->prev;
//             }
//             page_free(temp);
// 			list->count--;
//             return;
//         }
//         temp = temp->next;
//     }
// }

task_struct removeHead(task_linkedlist *list) {
    task_struct result = {0};  // 默认返回的task_struct，pid设为0表示无效
    if (list->head) {
        task_listnode *temp = list->head;
        result = temp->task;  // 获取要返回的task_struct
        list->head = list->head->next;
        if (list->head) {
            list->head->prev = NULL;
        } else {
            list->tail = NULL;
        }
        page_free(temp);
		list->count--;
    }
    return result;
}

task_struct removeTail(task_linkedlist *list) {
    task_struct result = {0};  // 默认返回的task_struct，pid设为0表示无效
    if (list->tail) {
        task_listnode *temp = list->tail;
        result = temp->task;  // 获取要返回的task_struct
        list->tail = list->tail->prev;
        if (list->tail) {
            list->tail->next = NULL;
        } else {
            list->head = NULL;
        }
        page_free(temp);
		list->count--;
    }
    return result;
}

task_struct removeByPid(task_linkedlist *list, uint32_t pid) {
    task_struct result = {0};  // 默认返回的task_struct，pid设为0表示无效
    task_listnode *temp = list->head;
    while (temp) {
        if (temp->task.pid == pid) {
            result = temp->task;  // 获取要返回的task_struct
            if (temp->prev) {
                temp->prev->next = temp->next;
            } else {
                list->head = temp->next;
            }
            if (temp->next) {
                temp->next->prev = temp->prev;
            } else {
                list->tail = temp->prev;
            }
            page_free(temp);
			list->count--;
            return result;
        }
        temp = temp->next;
    }
    return result;
}

task_listnode *findByPid(task_linkedlist *list, uint32_t pid) {
    task_listnode *current = list->head;
    while (current != NULL) {
        if (current->task.pid == pid) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

task_listnode *findByIndex(task_linkedlist *list, int index) {
    task_listnode *current = list->head;
    int currentIndex = 0;
    while (current != NULL) {
        if (currentIndex == index) {
            return current;
        }
        current = current->next;
        currentIndex++;
    }
    return NULL;  // 如果索引超出链表长度，返回NULL
}
