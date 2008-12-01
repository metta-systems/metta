//
// Copyright 2007 - 2008, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "MemoryManager.h"
#include "Registers.h"
#include "Kernel.h"

extern address_t end; // defined by linker.ld

namespace metta {
namespace kernel {

/**
* @internal
* Paging works by splitting the virtual address space into blocks
* called \c pages, which are usually 4KB in size. Pages can then
* be mapped on to \c frames - equally sized blocks of physical memory.
**/

memory_manager::memory_manager()
{
	placement_address = (address_t)&end ; // TODO: change to multiboot->mod_end
	heap_initialised = false;
	current_directory = kernel_directory = NULL;
}

memory_manager::~memory_manager()
{
}

void memory_manager::init(address_t mem_end)
{
	// Make enough frames to reach 0x00000000 .. memEnd.
    // make sure memEnd is on a page boundary.
	uint32_t memEndPage = mem_end - (mem_end % PAGE_SIZE);
	n_frames = memEndPage / PAGE_SIZE;

	frames = new bit_array(n_frames);

	// Make a page directory.
	kernel_directory = new(true/*page align*/) page_directory();
	current_directory = kernel_directory;

	// Map some pages in the kernel heap area.
	// Here we call getPage but not alloc_frame. This causes PageTables
	// to be created where nessecary. We can't allocate frames yet because
	// they need to be identity mapped first below.
	for (uint32_t i = HEAP_START; i < HEAP_END; i += PAGE_SIZE)
	{
		kernel_directory->getPage(i, /*make:*/true);
	}

	// Map some pages in the user heap area.
	// Here we call getPage but not alloc_frame. This causes PageTables
	// to be created where nessecary. We can't allocate frames yet because
	// they need to be identity mapped first below.
	for (uint32_t i = USER_HEAP_START; i < USER_HEAP_END; i += PAGE_SIZE)
	{
		kernel_directory->getPage(i, /*make:*/true);
	}

	// Identity map from KERNEL_START to placementAddress.
	uint32_t i = 0;
	while (i < placement_address)
	{
		// Kernel code is readable but not writable from userspace.
		alloc_frame(kernel_directory->getPage(i, true) , /*kernel:*/false, /*writable:*/false);
		i += PAGE_SIZE;
	}

// TODO BUG HERE!
	// Now allocate those pages we mapped earlier.
	for (i = HEAP_START; i < HEAP_START+HEAP_INITIAL_SIZE; i += PAGE_SIZE)
	{
		// Heap is readable but not writable from userspace.
		alloc_frame(kernel_directory->getPage(i, true), false, false);
	}

	for (i = USER_HEAP_START; i < USER_HEAP_START+USER_HEAP_INITIAL_SIZE; i += PAGE_SIZE)
	{
		alloc_frame(kernel_directory->getPage(i, true), false, true);
	}

	// write the page directory.
	write_page_directory((address_t)kernel_directory->getPhysical());
	enable_paging();

	// Initialise the heaps.
	heap_.init(HEAP_START, HEAP_START+HEAP_INITIAL_SIZE, HEAP_END & PAGE_MASK /* see memory map */, true);
	user_heap.init(USER_HEAP_START, USER_HEAP_START+USER_HEAP_INITIAL_SIZE, USER_HEAP_END & PAGE_MASK, false);

	heap_initialised = true;
}

void *memory_manager::malloc(uint32_t size, bool pageAlign, address_t *physicalAddress)
{
	ASSERT(heap_initialised);
	void *addr = heap_.allocate(size, pageAlign);
	if (physicalAddress)
	{
		page *pg = kernel_directory->getPage((address_t)addr, false);
		*physicalAddress = pg->frame() + (address_t)addr % PAGE_SIZE;
	}
	return addr;
}

void memory_manager::free(void *p)
{
	ASSERT(heap_initialised);
	heap_.free(p);
}

void *memory_manager::umalloc(uint32_t size)
{
	ASSERT(heap_initialised);
	return user_heap.allocate(size, false);
}

void memory_manager::ufree(void *p)
{
	ASSERT(heap_initialised);
	user_heap.free(p);
}

//
// alloc_frame -- maps a page to a frame.
//
void memory_manager::alloc_frame(page *p, bool isKernel, bool isWriteable)
{
	if (p->frame())
	{
		return;
	}
	else
	{
		// TODO: make this more efficient than O(n).
		uint32_t frameIdx = frames->first_clear();
		if (frameIdx == (uint32_t)-1)
		{
			PANIC("No free frames.");
		}

		frames->set(frameIdx);

		p->setPresent(true);
		p->setWritable(isWriteable);
		p->setUser(!isKernel);
		p->setFrame(frameIdx * PAGE_SIZE);
	}
}

//
// alloc_frame -- maps a page to a frame.
//
address_t memory_manager::alloc_frame()
{
	// TODO: make this more efficient than O(n).
	uint32_t frameIdx = frames->first_clear();
	if (frameIdx == (uint32_t)-1)
	{
		PANIC("No free frames.");
	}

	frames->set(frameIdx);

	return frameIdx * PAGE_SIZE;
}

void memory_manager::free_frame(page *p)
{
	uint32_t frame;
	if (!(frame = p->frame()))
	{
		return;
	}
	else
	{
		frames->clear(frame / PAGE_SIZE);
		p->setFrame(0x0);
	}
}

void memory_manager::free_frame(address_t frame)
{
	frames->clear(frame / PAGE_SIZE);
}

extern "C" address_t initialEsp; // in loader.s

void memory_manager::remap_stack()
{
	ASSERT(current_directory);

	uint32_t i;
	// Allocate some space for the stack.
	for (i = STACK_START; i > (STACK_START-STACK_INITIAL_SIZE); i -= PAGE_SIZE)
	{
		// General-purpose stack is in user-mode.
		alloc_frame(current_directory->getPage(i, /*make:*/true), /*kernel:*/false);
	}

	// Flush the TLB
	flush_page_directory();

	address_t oldStackPointer = read_stack_pointer();
	address_t oldBasePointer  = read_base_pointer();
	size_t stackSize = initialEsp - oldStackPointer;

	int offset = STACK_START - initialEsp;

	address_t newStackPointer = oldStackPointer + offset;
	address_t newBasePointer = oldBasePointer + offset;

	kconsole.print("Remapping stack from %p-%p to %p-%p (%d bytes)..", initialEsp, oldStackPointer, STACK_START, newStackPointer, stackSize);

	kernel::copy_memory((void*)newStackPointer, (const void*)oldStackPointer, stackSize);

	write_stack_pointer(newStackPointer);
	write_base_pointer(newBasePointer);
	kconsole.print("done\n");
}

void memory_manager::align_placement_address()
{
	if (placement_address % PAGE_SIZE) // if it needs aligning at all!
	{
		placement_address += PAGE_SIZE - placement_address % PAGE_SIZE;
	}
}

void memory_manager::allocate_range(address_t startAddress, address_t size)
{
	UNUSED(startAddress);
	UNUSED(size);
/*	Address endAddress = startAddress + size;
	processManager->getProcess()->memoryMap.map(startAddress, endAddress - startAddress, MemoryMap::Local);
	startAddress &= PAGE_MASK;
	endAddress   &= PAGE_MASK;

	for (Address i = startAddress; i <= endAddress; i += PAGE_SIZE)
	{
		alloc_frame( currentDirectory->getPage(i, true), false );
	}
	flushPageDirectory();*/
}

uint32_t memory_manager::get_kernel_heap_size()
{
	return heap_.getSize();
}

uint32_t memory_manager::get_user_heap_size()
{
	return user_heap.getSize();
}

void memory_manager::check_integrity()
{
	if(heap_initialised)
	{
		heap_.checkIntegrity();
		user_heap.checkIntegrity();
	}
}

}
}

// kate: indent-width 4; replace-tabs on;
// vi:set ts=4:set expandtab=on:
