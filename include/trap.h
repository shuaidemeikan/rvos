#ifndef _TRAP_H
#define _TRAP_H

#include "types.h"
#include "syscall.h"
#include "platform.h"
#include "sched.h"
#include "riscv.h"
#include "plic.h"
#include "sched.h"

void trap_init();
void external_interrupt_handler();
reg_t trap_handler(reg_t epc, reg_t cause, struct context *cxt);
void trap_test();

#endif // !_TRAP_H