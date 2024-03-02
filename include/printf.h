#ifndef _PRINTF_H
#define _PRINTF_H

#include <stddef.h>
#include <stdarg.h>
#include "types.h"
#include "uart.h"

int printf(const char* s, ...);
void panic(char *s);

#endif 
