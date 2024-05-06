#include "rand.h"

uint32_t randnum = 0;

void rand_init()
{
    randnum = 0x123456;
}

void rand_gen()
{
    uint8_t bytes[4];
    uint32_t result = 0;
    
    // 分解成四个8位的数
    for (int i = 0; i < 4; i++) {
        bytes[i] = (randnum >> (8 * (3 - i))) & 0xFF;
        
        // 处理每个8位数
        uint8_t x0 = bytes[i] & 0x01;
        uint8_t x2 = (bytes[i] >> 2) & 0x01;
        uint8_t x3 = (bytes[i] >> 3) & 0x01;
        uint8_t x4 = (bytes[i] >> 4) & 0x01;
        uint8_t x7 = x4 ^ x3 ^ x2 ^ x0;
        
        // 右移并设置新的x7
        bytes[i] = (bytes[i] >> 1) | (x7 << 7);
        
        // 重新组合为32位数
        result |= ((uint32_t)bytes[i] << (8 * (3 - i)));
    }
    
    randnum = result;
}

int rand()
{
    rand_gen();
    return randnum;
}