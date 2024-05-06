#include "virtmem.h"

mempage *task0_firstpage;

void mempage_init()
{
    // 初始化0号进程的页表
    // 目前只支持128M的内存，一个一级页表只能管理4M的内存，所以需要32个一级页表
    task0_firstpage = (mempage *)((uint32_t)page_alloc(2) + 2048);
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
        new_firstpage[i].W = 1; // 禁止写，启用写时复制
        new_firstpage[i].X = src_firstpage[i].X;
        new_firstpage[i].U = src_firstpage[i].U;
        new_firstpage[i].G = src_firstpage[i].G;
        new_firstpage[i].A = src_firstpage[i].A;
        new_firstpage[i].D = src_firstpage[i].D;
        new_firstpage[i].RSW = src_firstpage[i].RSW;
    }

    //将新创建的进程对应的地址写权限设为1
    uint32_t start_addr = 0x80000000;
    new_firstpage[0].W = 0;
    for (int i = 0; i < 32; i++)
    {
        if ((start_addr <= func_addr) && (func_addr < start_addr + 4194304))
        {
            new_firstpage[i].W = 0;
        }
        start_addr += 4194304;
    }

    // 二级页表的处理: 这里不分配新的二级页表，而是复用现有的二级页表，可以大幅度节约内存

    return new_firstpage;
}

// 分配一个新的二级页表到某个进程的一级页表的函数
void alloc_secpage(mempage *firstpage, uint32_t target_addr)
{
    // 找到target_addr对应的一级页表
    int page_index = -1;
    for (int i = 0; i < 32; i++) {
        if (0x80000000 + i * 4194304 <= target_addr && target_addr < 0x80000000 + (i + 1) * 4194304) {
            page_index = i;
            break;
        }
    }

    // 创建对应的二级页表，并复制一级页表对应的二级页表
    mempage *new_secpage = (mempage *)page_alloc(32);
    for (int i = 0; i < 1024; i++) {
        new_secpage[i].PPN = firstpage[page_index * 1024 + i].PPN;
        new_secpage[i].V = firstpage[page_index * 1024 + i].V;
        new_secpage[i].R = firstpage[page_index * 1024 + i].R;
        new_secpage[i].W = firstpage[page_index * 1024 + i].W;
        new_secpage[i].X = firstpage[page_index * 1024 + i].X;
        new_secpage[i].U = firstpage[page_index * 1024 + i].U;
        new_secpage[i].G = firstpage[page_index * 1024 + i].G;
        new_secpage[i].A = firstpage[page_index * 1024 + i].A;
        new_secpage[i].D = firstpage[page_index * 1024 + i].D;
        new_secpage[i].RSW = firstpage[page_index * 1024 + i].RSW;
    }

    // 修改一级页表的读写执行权限为0
    firstpage[page_index].W = 0;
    firstpage[page_index].R = 0;
    firstpage[page_index].X = 0;
}

// 以下代码是更精细的写时复制，但是问题在于我没有实现字节级别的内存分配器
// 如果要使用二级页表精度的内存权限控制，内存开销就太大了，还是使用上面的一级页表精度的内存权限控制吧
// mempage* get_second_level(mempage* firstpage, int index)
// {
//     if (index < 0 || index >= 32)
//         // 索引无效，返回 NULL
//         return NULL;

//     uint32_t ppn = firstpage[index].PPN;  // 获取物理页号
//     uint32_t physical_address = ppn * 4096;  // 计算物理地址，页大小为 4096 字节

//     // 将物理地址转换为 mempage 指针
//     mempage* second_level = (mempage*)physical_address;

//     return second_level;
// }



// mempage* fork_mempage(mempage* src_firstpage, void* func_addr)
// {
//     // 分配新的一级页表
//     mempage* new_firstpage = (mempage*)((uint32_t)page_alloc(2) + 2048);

//     // 复制一级页表
//     for (int i = 0; i < 32; i++)
//     {
//         new_firstpage[i] = src_firstpage[i];  // 复制所有属性
//     }

//     // 寻找新进程的起始地址
//     uint32_t start_addr = 0x80000000;
//     int page_size = 4194304;  // 每个一级页表对应的大小
//     int page_index = -1;

//     for (int i = 0; i < 32; i++)
//     {
//         if (start_addr <= (uint32_t)func_addr && (uint32_t)func_addr < start_addr + page_size)
//         {
//             page_index = i;
//             break;
//         }
//         start_addr += page_size;
//     }

//     if (page_index >= 0)
//     {
//         // 为对应的一级页表分配新的二级页表
//         mempage* new_secpage = (mempage*)page_alloc(32);  // 二级页表项数量
//         new_firstpage[page_index].PPN = (uint32_t)new_secpage / 4096;

//         // 从源一级页表中获取对应的二级页表
//         mempage* src_secpage = get_second_level(src_firstpage, page_index);

//         // 复制二级页表并设置写权限
//         for (int j = 0; j < 1024; j++)
//         {
//             new_secpage[j] = src_secpage[j];  // 复制属性
//             new_secpage[j].R = 0;  // 禁止读权限
//             new_secpage[j].W = 0;  // 禁止写权限
//             new_secpage[j].X = 0;  // 禁止执行权限
//         }

//         // 在新的二级页表中设置特定的页表项允许写
//         uint32_t offset = ((uint32_t)func_addr - start_addr) / 4096;
//         new_secpage[offset].W = 1;  // 允许写权限
//     }

//     return new_firstpage;
// }

void set_mempage(mempage *page)
{
    reg_t tmp = 0;
    tmp = tmp | 1U << 31 | ((((uint32_t)page - 2048) / 4096) & 0x3FFFFF);
    w_satp(tmp);
}
