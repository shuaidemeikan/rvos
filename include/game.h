#ifndef GAME_H
#define GAME_H

#include "stdio.h"
#include "memtools.h"
#include "page.h"
#include "rand.h"

// 因为需要设置创建进程的父进程，所以需要extern task_struct cur_task这个结构，这里可以简化，但是妹必要。
typedef struct _task_struct
{
	char tss[128];    		// 进程的寄存器信息
    uint32_t state;         // 0就绪，1因调用了延迟函数在等待
	int delay_time;			// -1为不等，其他为等待的时间
	void* start_pc;			// 进程的入口地址
    uint32_t counter;       // 时间片计数
    uint32_t priority;      // 优先级
    uint32_t exit_code;     // 进程执行停止的退出码，父进程会取
    uint32_t pid;           // 进程号
    uint32_t father;        // 父进程号
    uint32_t tty;           // 可能有用，进程使用的tty设备号   
    // 还应该有文件系统的东西，ss暂时没做
}task_struct;

#define ANSI_COLOR_RED   31
#define ANSI_COLOR_GREEN 32
#define ANSI_COLOR_WHITE 37
#define ANSI_COLOR_RESET 0

// 以下是游戏正常运行需要的api
extern void task_delay(volatile int count);

static inline void set_color(int c) {
  printf("\033[%dm", c);
}

static inline void print_char(char c, int y, int x) {
  printf("\033[%d;%dH%c", y + 1, x + 1, c);
}

static inline void screen_clear() {
  printf("\033[H\033[J");
}

static inline void screen_refresh() {
}

static inline int screen_tile_height() {
  return 24;
}

static inline int screen_tile_width() {
  return 80;
}

#define usleep(x) task_delay((x / 1000) * 25)
#define malloc(x) page_alloc(1)

void printaandb();
void hanoi();
void game_of_life(void);
void donut(void);

#endif