#include "memtools.h"
void* memcpy(void* ptr1, const void* ptr2, size_t len)
{
    for (int i = 0; i < len; i++)
        ((char*)ptr1)[i] = ((char*)ptr2)[i];
    return ptr1;
}

int memcmp(const void *s1, const void *s2, size_t n) {
  for (int i = 0; i < n; i++)
  {
    if (((char*)s1)[i] < ((char*)s2)[i])
      return -1;
    else if(((char*)s1)[i] > ((char*)s2)[i])
       return 1;
  }
  return 0;
}


void* memset(void *s, int c, size_t n)
{
    for (int i = 0; i < n; i++)
        ((char*)s)[i] = (char)c;
    return s;
}

void ptr2uint(uint32_t* target, void* ptr)
{
    memcpy(target, ptr, sizeof(uint32_t));
}