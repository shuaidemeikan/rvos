#include "rvtime.h"
#include "sched.h"

void usleep(uint32_t us)
{
    extern void systimedelay(uint32_t us);
    systimedelay(us);
}

void timer_delay(uint32_t ms)
{
    extern task_struct cur_task;
    cur_task.delay_time = ms;
    cur_task.state = 1;
    schedule();
}