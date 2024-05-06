#ifndef VIRTMEM_H
#define VIRTMEM_H

#include "types.h"
#include "memtools.h"
#include "page.h"
#include "riscv.h"

#pragma pack(1)

typedef struct _mempage
{
    uint32_t V : 1;    // 有效位（Valid）
    uint32_t R : 1;    // 可读位（Read）
    uint32_t W : 1;    // 可写位（Write）
    uint32_t X : 1;    // 可执行位（Execute）
    uint32_t U : 1;    // 用户模式位（User）
    uint32_t G : 1;    // 全局位（Global）
    uint32_t A : 1;    // 访问位（Accessed）
    uint32_t D : 1;    // 脏位（Dirty）
    uint32_t RSW : 2;  // 保留供软件使用的位（Reserved for Software Use）
    uint32_t PPN : 20; // 物理页号（Physical Page Number）
}mempage;


#pragma pack()

void mempage_init();
mempage *fork_mempage(mempage *src_firstpage, void* func_addr);
void set_mempage(mempage *page);
void alloc_secpage(mempage *firstpage, uint32_t target_addr);

#endif 