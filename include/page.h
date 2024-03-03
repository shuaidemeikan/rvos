#ifndef _PAGE_H
#define _PAGE_H

#include "types.h"
#include <stddef.h>
#include "printf.h"


#define PAGE_SIZE 4096
#define PAGE_ORDER 12

#define PAGE_TAKEN (uint8_t)(1 << 0)
#define PAGE_LAST  (uint8_t)(1 << 1)

#define byte_alloc_digit digit if (size < digit)\
	{\
		memory_bin* curr = byte_alloclist->memdigit;\
		while (curr->next != NULL)\
		{\
			if (curr->full == 0)\
				return curr;\
			else\
				curr = curr->next;\
		}\
		for (int i = 0; i < 256; i++)\
		{\
			if (curr->exist[i] == 0)\
			{\
				curr->exist[i] = 1;\
				curr->allcated_count++;\
				if (curr->allcated_count == 256)\
				{\
					curr->full = 1;\
					if (get_binisfull(byte_alloclist->memdigit) == NULL)\
					{\
						memory_bin* newbin = page_alloc(curr->page_size);\
						newbin->page_size = (digit * 256) / 4096;\
						newbin->start_addr = page_alloc(newbin->page_size);\
						newbin->pre = curr;\
						newbin->next = NULL;	\
						newbin->full = 0;\
						newbin->allcated_count = 0;\
						addbintolink(curr, newbin);\
						byte_alloclist->memdigit_count++;\
					}\
				}\
				return curr->start_addr + i * digit;\
			}\
		}\
	}\

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

typedef struct _memory_bin
{
	uint8_t exist[256];
	void* start_addr;
	uint16_t page_size;
	struct _memory_bin* pre;
	struct _memory_bin* next;
	uint8_t full;
	uint16_t allcated_count;
}memory_bin;

typedef struct _byte_memory
{
	memory_bin* mem64;
	uint16_t mem64_count;
	memory_bin* mem128;
	uint16_t mem128_count;
}byte_memory;

#pragma pack()

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

static inline memory_bin* get_binisfull(memory_bin* bin)
{
	while (bin != NULL)
	{
		if (bin->full == 0)
			return bin;
		else
			bin = bin->next;
	}
	return NULL;
}

static inline void addbintolink(memory_bin* link, memory_bin* bin)
{
	if (link->next == NULL)
	{
		link->next = bin;
		bin->pre = link;
		bin->next = NULL;
	}else
	{
		link->next->pre = bin;
		bin->next = link->next;
		bin->pre = link;
		link->next = bin;
	}
}

static inline void free_bin(memory_bin* bin)
{
	if (bin->pre == NULL)
		return;
	if (bin->next == NULL)
	{
		bin->pre->next = NULL;
		for (int i = 0; i < bin->page_size; i++)
			page_free(bin->start_addr + i * PAGE_SIZE);
		page_free(bin);
	}else
	{
		bin->pre->next = bin->next;
		bin->next->pre = bin->pre;
		for (int i = 0; i < bin->page_size; i++)
			page_free(bin->start_addr + i * PAGE_SIZE);
		page_free(bin);
	}
}

#endif