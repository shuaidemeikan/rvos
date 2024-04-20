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
//task_struct ready_task[MAX_TASKS];
task_linkedlist* ready_task;
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

	ready_task = initLinkedList();

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
		//cur_task = removeHead(ready_task);
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

	//_current = (_current + 1) % _top;
	//cur_task = ready_task[_current];

	// 首先保存当前上下文
	if (cur_task.pid != -1)
		addTail(ready_task, cur_task);
	// 从队列首部移出一个上下文用以切换
	cur_task = removeHead(ready_task);
	cur_task.counter = 5;
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

static void task_loader(task_struct task, int argc, char **argv)
{
	// 调用被加载的函数
	int retvalue = ((int (*)(int, char **))(task.tss.pc))(argc, argv);

	// 进程执行完后，先删除在ready队列中删除进程
	removeByPid(ready_task, task.pid);
	// 再返回到sched函数，进行进程切换
	asm volatile("mv x1, %0"::"r"(schedule));
	return;
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

static int progress1(int argc, char **argv)
{
   	//while (1)
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
	return 1;
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
	//ready_task[_top] = *task0;
	addTail(ready_task, *task0);
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
	//ready_task[_top] = *task01;
	addTail(ready_task, *task01);
	_top++;
}

int task_create(void* start_routin, uint32_t counter, uint32_t priority, uint32_t tty, uint32_t father)
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

task_linkedlist *initLinkedList() {
    task_linkedlist *list = (task_linkedlist *)byte_alloc(sizeof(task_linkedlist));
    if (list) {
        list->head = NULL;
        list->tail = NULL;
    }
	list->count = 0;
    return list;
}

void addHead(task_linkedlist *list, task_struct task) {
    task_listnode *node = (task_listnode *)byte_alloc(sizeof(task_listnode));
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
    task_listnode *node = (task_listnode *)byte_alloc(sizeof(task_listnode));
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
        task_listnode *node = (task_listnode *)byte_alloc(sizeof(task_listnode));
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
//         byte_free(temp);
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
//         byte_free(temp);
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
//             byte_free(temp);
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
        byte_free(temp);
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
        byte_free(temp);
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
            byte_free(temp);
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