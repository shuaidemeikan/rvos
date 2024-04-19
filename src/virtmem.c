#include "virtmem.h"

void mempage_init()
{
    // 初始化0号进程的页表
    // 目前只支持128M的内存，一个一级页表只能管理4M的内存，所以需要32个一级页表
    mempage *task0_firstpage = (mempage *)((uint32_t)page_alloc(2) + 2048);
    // mempage* task0_firstpage =  (mempage*)((uint32_t)page_alloc(2));
    //  有32个一级页表，一个一级页表有1024个二级页表项，所以二级页表需要的总空间为
    //  32 * 1024 * 32
    mempage *task0_secpage = (mempage *)page_alloc(256);
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
    // task0_firstpage[0].PPN = 0x80000000 / 4096;

    // 初始化二级页表
    for (int i = 0; i < 1024 * 32; i++)
    {
        task0_secpage[i].PPN = (0x80000000 + i * 4096) / 4096;
        // task0的页表比较特殊，这里假设它所有页表项都可读写执行
        // 脏读和脏写位如果不用设置为1在硬件的角度上是最快的
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

mempage *fork_mempage(mempage *src_firstpage, void* func_addr) {
    // 分配新的一级页表
    mempage *new_firstpage = (mempage *)((uint32_t)page_alloc(2) + 2048);

    // 复制一级页表中的页表项，同时设置写时复制标志
    for (int i = 0; i < 32; i++) {
        new_firstpage[i].PPN = src_firstpage[i].PPN;
        new_firstpage[i].V = src_firstpage[i].V;
        new_firstpage[i].R = src_firstpage[i].R;
        new_firstpage[i].W = 0; // 禁止写，启用写时复制
        new_firstpage[i].X = src_firstpage[i].X;
        new_firstpage[i].U = src_firstpage[i].U;
        new_firstpage[i].G = src_firstpage[i].G;
        new_firstpage[i].A = src_firstpage[i].A;
        new_firstpage[i].D = src_firstpage[i].D;
        new_firstpage[i].RSW = src_firstpage[i].RSW;
    }

    // 将新创建的进程对应的地址写权限设为1
    for (int i = 0; i < 32; i++)
    {
        int start_addr = 0x80000000;
        if (start_addr <= func_addr && func_addr < start_addr + 4194304);
        {
            new_firstpage[i].W = 1;
        }
        start_addr += 4194304;
    }

    // 二级页表的处理: 这里不分配新的二级页表，而是复用现有的二级页表，可以大幅度节约内存

    return new_firstpage;
}

void set_mempage(mempage *page)
{
    reg_t tmp = 0;
    tmp = tmp | 1U << 31 | ((((uint32_t)page - 2048) / 4096) & 0x3FFFFF);
    w_satp(tmp);
}
