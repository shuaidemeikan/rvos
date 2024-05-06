// System call numbers
#ifndef _SYSCALL_H
#define _SYSCALL_H

#include "sched.h"
#include <stddef.h>
#include "printf.h"


#define SYS_gethid	    1
#define SYS_printf      2
#define SYS_endsched    3
#define SYS_timedelay   4

int sys_gethid(unsigned int *ptr_hid);
void do_syscall(struct context *cxt);

#endif // !_SYSCALL_H

