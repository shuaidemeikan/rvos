#include "stdio.h"

stdio_uart_fifo* put_buf;

void stdio_init()
{
    put_buf = (stdio_uart_fifo*)page_alloc(1);
    fifo_init(put_buf);
}

char getc()
{
    char c;
    while (!fifo_dequeue(put_buf, &c));
    return c;
}

char* gets(char *s)
{
    char *p = s;
    char c;
    while ((c = getc()) != '\r')
    {
        *p++ = c;
    }
    *p = '\0';
    return s;
}

void global_put(char c)
{	
	while (!fifo_enqueue(put_buf, c));	
}

// 初始化FIFO
void fifo_init(stdio_uart_fifo *fifo) {
    fifo->head = 0;
    fifo->tail = 0;
    fifo->count = 0;
}

// 检查FIFO是否为空
int fifo_is_empty(const stdio_uart_fifo *fifo) {
    return fifo->count == 0;
}

// 检查FIFO是否已满
int fifo_is_full(const stdio_uart_fifo *fifo) {
    return fifo->count == FIFO_SIZE;
}

// 从FIFO中取出一个字符
int fifo_dequeue(stdio_uart_fifo *fifo, char *c) {
    //spin_lock();
    if (fifo_is_empty(fifo)) {
        //printf("fifo is empty\n");
        return 0;
    }

    *c = fifo->buffer[fifo->tail]; // 从缓冲区取出字符
    fifo->tail = (fifo->tail + 1) % FIFO_SIZE; // 循环移动尾指针
    fifo->count--; // 减少元素数量
    //spin_unlock();
    return 1;
}


// 往FIFO中存入一个字符
int fifo_enqueue(stdio_uart_fifo *fifo, char c) {
    //spin_lock();
    if (fifo_is_full(fifo)) {
        //printf("fifo is full\n");
		// 这里做一个特殊处理，其实应该用ringbuf而不是fifo的
		// 但是懒得改了，如果fifo满了就从fifo里取出来一个，这样就实现了一个变相的ringbuf
		char tmp;
		fifo_dequeue(fifo, &tmp);		
    }

fifo->buffer[fifo->head] = c; // 将字符存入缓冲区
    fifo->head = (fifo->head + 1) % FIFO_SIZE; // 循环移动头指针
    fifo->count++; // 增加元素数量
    //spin_unlock();
    return 1;
}
