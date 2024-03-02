#ifndef _PAGE_H
#define _PAGE_H

#include "types.h"
#include <stddef.h>
#include "printf.h"


#define PAGE_SIZE 4096
#define PAGE_ORDER 12

#define PAGE_TAKEN (uint8_t)(1 << 0)
#define PAGE_LAST  (uint8_t)(1 << 1)

/*
 * Page Descriptor 
 * flags:
 * - bit 0: flag if this page is taken(allocated)
 * - bit 1: flag if this page is the last page of the memory block allocated
 */
struct Page {
	uint8_t flags;
};

#pragma pack(1)
typedef struct _byte_memory
{
	void* start_addr;
	uint16_t len;
	uint16_t exist;
	struct _byte_memory pre;
	struct _byte_memory next;
} byte_memory;
#pragma pack()

static inline byte_memory* get_memblk(byte_memory* list, uint32_t list_len)
{
	byte_memory* curr = list;
	for (int i = 0; i < list_len; i++)
	{
		if (list->exist == 0)
			return curr;	
		curr++;
	}
	return NULL;
}

static inline void del_memblk(byte_memory* node)
{
	node->exist = 0;
	
}

static inline void _clear(struct Page *page)
{
	page->flags = 0;
}

static inline int _is_free(struct Page *page)
{
	if (page->flags & PAGE_TAKEN) {
		return 0;
	} else {
		return 1;
	}
}

static inline void _set_flag(struct Page *page, uint8_t flags)
{
	page->flags |= flags;
}

static inline int _is_last(struct Page *page)
{
	if (page->flags & PAGE_LAST) {
		return 1;
	} else {
		return 0;
	}
}

/*
 * align the address to the border of page(4K)
 */
static inline uint32_t _align_page(uint32_t address)
{
	uint32_t order = (1 << PAGE_ORDER) - 1;
	return (address + order) & (~order);
}

void page_init();
void *page_alloc(int npages);
void page_free(void *p);
void page_test();

#endif