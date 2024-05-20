#include "memtools.h"
void* memcpy(void* ptr1, const void* ptr2, size_t len)
{
    for (int i = 0; i < len; i++)
        ((char*)ptr1)[i] = ((char*)ptr2)[i];
    return ptr1;
}

// int memcmp(const void *s1, const void *s2, size_t n) {
//   for (int i = 0; i < n; i++)
//   {
//     if (((char*)s1)[i] < ((char*)s2)[i])
//       return -1;
//     else if(((char*)s1)[i] > ((char*)s2)[i])
//        return 1;
//   }
//   return 0;
// }

int memcmp(const void *s1, const void *s2, size_t n) {
    const uint8_t *byte1 = s1;
    const uint8_t *byte2 = s2;
    int result = 0;

    __asm__ volatile (
        "1:\n"
        "lb    %[result], 0(%[byte1])\n"
        "lb    t0, 0(%[byte2])\n"
        "sub   %[result], %[result], t0\n"
        "bnez  %[result], 2f\n"
        "addi  %[byte1], %[byte1], 1\n"
        "addi  %[byte2], %[byte2], 1\n"
        "addi  %[n], %[n], -1\n"
        "bnez  %[n], 1b\n"
        "2:\n"
        : [result] "=r" (result), [byte1] "+r" (byte1), [byte2] "+r" (byte2), [n] "+r" (n)
        : 
        : "t0", "memory"
    );

    return result;
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