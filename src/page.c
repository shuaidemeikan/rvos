#include "page.h"

/*
 * Following global vars are defined in mem.S
 */
extern uint32_t TEXT_START;
extern uint32_t TEXT_END;
extern uint32_t DATA_START;
extern uint32_t DATA_END;
extern uint32_t RODATA_START;
extern uint32_t RODATA_END;
extern uint32_t BSS_START;
extern uint32_t BSS_END;
extern uint32_t HEAP_START;
extern uint32_t HEAP_SIZE;

/*
 * _alloc_start points to the actual start address of heap pool
 * _alloc_end points to the actual end address of heap pool
 * _num_pages holds the actual max number of pages we can allocate.
 */
static uint32_t _alloc_start = 0;
static uint32_t _alloc_end = 0;
static uint32_t _num_pages = 0;

static byte_memory* byte_alloclist;

// static byte_memory* byte_allocfraglist;
// static uint32_t byte_allocfrag_maxnum = 4096; 
// static uint32_t curr_byte_allocfrag = 0;


void page_init()
{
	/* 
	 * We reserved 8 Page (8 x 4096) to hold the Page structures.
	 * It should be enough to manage at most 128 MB (8 x 4096 x 4096) 
	 */
	_num_pages = (HEAP_SIZE / PAGE_SIZE) - 8;
	printf("HEAP_START = %x, HEAP_SIZE = %x, num of pages = %d\n", HEAP_START, HEAP_SIZE, _num_pages);
	
	struct Page *page = (struct Page *)HEAP_START;
	for (int i = 0; i < _num_pages; i++) {
		_clear(page);
		page++;	
	}

	_alloc_start = _align_page(HEAP_START + 8 * PAGE_SIZE);
	_alloc_end = _alloc_start + (PAGE_SIZE * _num_pages);

	printf("TEXT:   0x%x -> 0x%x\n", TEXT_START, TEXT_END);
	printf("RODATA: 0x%x -> 0x%x\n", RODATA_START, RODATA_END);
	printf("DATA:   0x%x -> 0x%x\n", DATA_START, DATA_END);
	printf("BSS:    0x%x -> 0x%x\n", BSS_START, BSS_END);
	printf("HEAP:   0x%x -> 0x%x\n", _alloc_start, _alloc_end);

	memory_bin* bin64 = (memory_bin*)page_alloc(1);
	bin64->page_size = (64 * 256) / 4096;
	bin64->start_addr = page_alloc(bin64->page_size);
	bin64->pre = NULL;
	bin64->next = NULL;	
	bin64->full = 0;
	bin64->allcated_count = 0;
	memory_bin* bin128 = (memory_bin*)page_alloc(1);
	bin128->page_size = (128 * 256) / 4096;
	bin128->start_addr = page_alloc(bin128->page_size);
	bin128->pre = NULL;
	bin128->next = NULL;	
	bin128->full = 0;
	bin128->allcated_count = 0;
	byte_alloclist = (byte_memory*)page_alloc(1);
	byte_alloclist->mem64 = bin64;
	byte_alloclist->mem128 = bin128;
	byte_alloclist->mem64_count = 0;
	byte_alloclist->mem128_count = 0;
}

/*
 * Allocate a memory block which is composed of contiguous physical pages
 * - npages: the number of PAGE_SIZE pages to allocate
 */
void *page_alloc(int npages)
{
	/* Note we are searching the page descriptor bitmaps. */
	int found = 0;
	struct Page *page_i = (struct Page *)HEAP_START;
	for (int i = 0; i <= (_num_pages - npages); i++) {
		if (_is_free(page_i)) {
			found = 1;
			/* 
			 * meet a free page, continue to check if following
			 * (npages - 1) pages are also unallocated.
			 */
			struct Page *page_j = page_i + 1;
			for (int j = i + 1; j < (i + npages); j++) {
				if (!_is_free(page_j)) {
					found = 0;
					break;
				}
				page_j++;
			}
			/*
			 * get a memory block which is good enough for us,
			 * take housekeeping, then return the actual start
			 * address of the first page of this memory block
			 */
			if (found) {
				struct Page *page_k = page_i;
				for (int k = i; k < (i + npages); k++) {
					_set_flag(page_k, PAGE_TAKEN);
					page_k++;
				}
				page_k--;
				_set_flag(page_k, PAGE_LAST);
				return (void *)(_alloc_start + i * PAGE_SIZE);
			}
		}
		page_i++;
	}
	return NULL;
}

/*
 * Free the memory block
 * - p: start address of the memory block
 */
void page_free(void *p)
{
	/*
	 * Assert (TBD) if p is invalid
	 */
	if (!p || (uint32_t)p >= _alloc_end) {
		return;
	}
	/* get the first page descriptor of this memory block */
	struct Page *page = (struct Page *)HEAP_START;
	page += ((uint32_t)p - _alloc_start)/ PAGE_SIZE;
	/* loop and clear all the page descriptors of the memory block */
	while (!_is_free(page)) {
		if (_is_last(page)) {
			_clear(page);
			break;
		} else {
			_clear(page);
			page++;;
		}
	}
}

void* byte_alloc(size_t size)
{
	if (size < 64)
	{
		memory_bin* curr = byte_alloclist->mem64;
		// 拿到第一个还有空位的bin
		// 这里理论存在两个情况，分别是找到一个有空位的bin和所有链上的bin都没有空位
		// 但是实际上第二种情况不会出现，因为当一个链满了的时候，我们会实时的给他分配一个新的bins
		while (curr->next != NULL)
		{
			if (curr->full == 0)
				return curr;
			else
				curr = curr->next;
		}
		// 找到空位的那一项
		for (int i = 0; i < 256; i++)
		{
			if (curr->exist[i] == 0)
			{
				curr->exist[i] = 1;
				curr->allcated_count++;
				if (curr->allcated_count == 256)
				{
					// 当前的bin里的256个空间全部被使用了
					curr->full = 1;
					if (get_binisfull(byte_alloclist->mem64) == NULL)
					{	// 如果整条链上都没有可使用的空间了，那么就要申请一个新的bin
						memory_bin* newbin = page_alloc(curr->page_size);
						newbin->page_size = (64 * 256) / 4096;
						newbin->start_addr = page_alloc(newbin->page_size);
						newbin->pre = curr;
						newbin->next = NULL;	
						newbin->full = 0;
						newbin->allcated_count = 0;
						addbintolink(curr, newbin);
						byte_alloclist->mem64_count++;
					}
				}
				return curr->start_addr + i * 64;
			}
		}
	}
	return NULL;
}

void byte_free(void* p)
{
	// 找到这个内存是属于哪个bin的
	// 先找64字节的bins
	memory_bin* curr = byte_alloclist->mem64;
	while(curr != NULL)
	{
		if (curr->start_addr <= p && p < curr->start_addr + PAGE_SIZE)
			break;
		else
			curr = curr->next;
	}
	if (curr != NULL)
	{
		// 说明在64字节处找到了，接下来找属于这个bin的哪一位
		for (int i = 0; i < 256; i++)
		{
			if (curr->start_addr + i * 64 == p)
			{
				curr->exist[i] = 0;
				curr->allcated_count--;
				if (curr->allcated_count == 0)
				{
					// 分配完了，把这个bin和bin占用的page回收
					free_bin(curr);
					byte_alloclist->mem64_count--;
				}
				return;
			}
		}
		printf("err: byte memory free err");
	}
}

void page_test()
{
	void *p = page_alloc(2);
	printf("p = 0x%x\n", p);
	//page_free(p);

	void *p2 = page_alloc(7);
	printf("p2 = 0x%x\n", p2);
	page_free(p2);

	void *p3 = page_alloc(4);
	printf("p3 = 0x%x\n", p3);
}

void byte_test()
{
	for (int i = 0; i < 256; i++)
	{
		byte_alloc(sizeof(int));
	}
	void* tmp = byte_alloc(sizeof(int));
	byte_free(tmp);
}