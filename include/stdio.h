#ifndef STDIO_H
#define STDIO_H

#include "printf.h"
#include "page.h"
#include "lock.h"

#define FIFO_SIZE 30 // 队列的深度

typedef struct _stdio_uart_fifo{
    char buffer[FIFO_SIZE]; // 队列缓冲区
    int head;               // 头指针，指向队列的下一个写入位置
    int tail;               // 尾指针，指向队列的下一个读取位置
    int count;              // 队列中的元素数量
}stdio_uart_fifo;

char getc();
char* gets(char *s);
void global_put(char c);

void fifo_init(stdio_uart_fifo *fifo);
int fifo_is_empty(const stdio_uart_fifo *fifo);
int fifo_is_full(const stdio_uart_fifo *fifo);
int fifo_enqueue(stdio_uart_fifo *fifo, char c);
int fifo_dequeue(stdio_uart_fifo *fifo, char *c);


#endif // STDIO_H
