#include "memtools.h"
void* memcpy(void* ptr1, const void* ptr2, size_t len)
{
    for (int i = 0; i < len; i++)
        ((char*)ptr1)[i] = ((char*)ptr2)[i];
    return ptr1;
}

int memcmp(const void *ptr1, const void *ptr2, size_t num)
{
    for (int i = 0; i < num; i++)
    {
        if (((char*)ptr1)[i] < ((char*)ptr2)[i])
            return -1;
        else if(((char*)ptr1)[i] > ((char*)ptr2)[i])
            return 1;
    }
    return 0;
}

void* memset(void *s, int c, size_t n)
{
    for (int i = 0; i < n; i++)
        ((int*)s)[i] = c;
    return s;
}