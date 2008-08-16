#pragma once
#ifndef __INCLUDED_KALLOC_H
#define __INCLUDED_KALLOC_H

// Simple kernel memory allocator.
#include "common.h"

#define KHEAP_START         0xC0000000
#define KHEAP_INITIAL_SIZE  0x100000

/**
   Allocate a chunk of memory, sz in size. The chunk must be
   page aligned.
**/
uint32_t kmalloc_a(uint32_t sz);

/**
   Allocate a chunk of memory, sz in size. The physical address
   is returned in phys. Phys MUST be a valid pointer to uint32_t!
**/
uint32_t kmalloc_p(uint32_t sz, uint32_t *phys);

/**
   Allocate a chunk of memory, sz in size. The physical address
   is returned in phys. It must be page-aligned.
**/
uint32_t kmalloc_ap(uint32_t sz, uint32_t *phys);

/**
   General allocation function.
**/
uint32_t kmalloc(uint32_t sz);

/**
   General deallocation function.
**/
void kfree(uint32_t p);

// OrderedArray needs declarations of kmalloc/kfree itself.
#include "OrderedArray.h"

class DefaultHeap
{
	public:
		/**
			Create a new heap.
		**/
		DefaultHeap(uint32_t start, uint32_t end, uint32_t max, bool supervisor, bool readonly);

		/**
			Allocates a contiguous region of memory 'size' in size. If page_align==1, it creates that block starting
			on a page boundary.
		**/
		void *alloc(uint32_t size, bool page_align);

		/**
			Releases a block allocated with 'alloc'.
		**/
		void free(void *p);

	private:
		int32_t find_smallest_hole(uint32_t size, uint8_t page_align);
		void expand(uint32_t new_size);
		uint32_t contract(uint32_t new_size);

	private:
		/**
			Size information for a hole/block
		**/
		struct HeapHeader
		{
			uint32_t magic;   // Magic number, used for error checking and identification.
			uint8_t  is_hole; // 1 if this is a hole. 0 if this is a block.
			uint32_t size;    // size of the block, including the end footer.

			inline int operator < (const HeapHeader &b)
			{
				return size < b.size;
			}
		};

		struct HeapFooter
		{
			uint32_t magic;     // Magic number, same as in header_t.
			HeapHeader *header; // Pointer to the block header.
		};

		OrderedArray<HeapHeader> index;
		uint32_t start_address; // The start of our allocated space.
		uint32_t end_address;   // The end of our allocated space. May be expanded up to max_address.
		uint32_t max_address;   // The maximum address the heap can be expanded to.
		bool supervisor;     // Should extra pages requested by us be mapped as supervisor-only?
		bool readonly;       // Should extra pages requested by us be mapped as read-only?
};

#endif
