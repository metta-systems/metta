#include "kalloc.h"

// end is defined in the linker script.
extern uint32_t end;
uint32_t placement_address = (uint32_t)&end;

/**
	\internal
	Allocate a chunk of memory, sz in size. If align == 1,
	the chunk must be page-aligned. If phys != 0, the physical
	location of the allocated chunk will be stored into phys.

	This is the internal version of kmalloc. More user-friendly
	parameter representations are available in kmalloc, kmalloc_a,
	kmalloc_ap, kmalloc_p.
**/
static uint32_t kmalloc_internal(uint32_t sz, int align, uint32_t *phys)
{
	// This will eventually call malloc() on the kernel heap.
	// For now, though, we just assign memory at placement_address
	// and increment it by sz. Even when we've coded our kernel
	// heap, this will be useful for use before the heap is initialised.
	if (align == 1 && (placement_address & 0xFFFFF000) )
	{
		// Align the placement address;
		placement_address &= 0xFFFFF000;
		placement_address += 0x1000;
	}
	if (phys)
	{
		*phys = placement_address;
	}
	uint32_t tmp = placement_address;
	placement_address += sz;
	return tmp;
}

uint32_t kmalloc_a(uint32_t sz)
{
	return kmalloc_internal(sz, 1, 0);
}

uint32_t kmalloc_p(uint32_t sz, uint32_t *phys)
{
	return kmalloc_internal(sz, 0, phys);
}

uint32_t kmalloc_ap(uint32_t sz, uint32_t *phys)
{
	return kmalloc_internal(sz, 1, phys);
}

uint32_t kmalloc(uint32_t sz)
{
	return kmalloc_internal(sz, 0, 0);
}
