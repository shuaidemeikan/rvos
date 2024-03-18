#include "virtmem.h"

void mempage_init()
{
    // 初始化0号进程的页表
    // 目前只支持128M的内存，一个一级页表只能管理4M的内存，所以需要32个一级页表
    mempage* task0_firstpage =  (mempage*)((uint32_t)page_alloc(2) + 2048);
    //mempage* task0_firstpage =  (mempage*)((uint32_t)page_alloc(2));
    // 有32个一级页表，一个一级页表有1024个二级页表项，所以二级页表需要的总空间为
    // 32 * 1024 * 32
    mempage* task0_secpage = (mempage*)page_alloc(256);
    // 初始化一级页表
    for (int i = 0; i < 32; i++)
    {
        if ((uint32_t)(&task0_secpage[i * 1024]) % 4096 != 0)
            printf("err, i = %d", i);
        task0_firstpage[i].PPN = (uint32_t)(&task0_secpage[i * 1024]) / 4096;
        task0_firstpage[i].V = 1;
        task0_firstpage[i].R = 0;
        task0_firstpage[i].W = 0;
        task0_firstpage[i].X = 0;
        task0_firstpage[i].U = 0;
        task0_firstpage[i].G = 0;
        task0_firstpage[i].A = 1;
        task0_firstpage[i].D = 1;
        task0_firstpage[i].RSW = 0;
    }
    //task0_firstpage[0].PPN = 0x80000000 / 4096;

    // 初始化二级页表
    for (int i = 0; i < 1024 * 32; i++)
    {
        task0_secpage[i].PPN = (0x80000000 + i * 4096) / 4096;
        // task0的页表比较特殊，这里假设它所有页表项都可读写执行
        task0_secpage[i].V = 1;
        task0_secpage[i].R = 1;
        task0_secpage[i].W = 1;
        task0_secpage[i].X = 1;
        task0_secpage[i].U = 0;
        task0_secpage[i].G = 0;
        task0_secpage[i].A = 1;
        task0_secpage[i].D = 1;
        task0_secpage[i].RSW = 0; 
    }

    // 开启分页
    // ASID字段好像是优化用的，但是不管也行
    reg_t tmp = 0;
    tmp = tmp | 1U << 31 | ((((uint32_t)task0_firstpage - 2048) / 4096) & 0x3FFFFF);
    w_satp(tmp);
}