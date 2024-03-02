#ifndef _MEMTOOLS_H
#define _MEMTOOLS_H
#include "types.h"


void* memcpy(void* ptr1, const void* ptr2, size_t len);
int memcmp(const void *ptr1, const void *ptr2, size_t num);
void* memset(void *s, int c, size_t n);

#endif // !_MEMTOOLS_H