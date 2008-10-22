//
// Copyright 2007 - 2008, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "MemoryManager.h"
#include "Registers.h"
#include "Kernel.h"

extern address_t end; // defined by linker.ld

/**
 * @internal
 * Paging works by splitting the virtual address space into blocks
 * called \c pages, which are usually 4KB in size. Pages can then
 * be mapped on to \c frames - equally sized blocks of physical memory.
 */

MemoryManager::MemoryManager()
{
	placementAddress = (address_t)&end ; // TODO: change to multiboot->mod_end
	heapInitialised = false;
	currentDirectory = kernelDirectory = NULL;
}

MemoryManager::~MemoryManager()
{
}

void MemoryManager::init(address_t memEnd)
{
	// Make enough frames to reach 0x00000000 .. memEnd.
	uint32_t memEndPage = memEnd - (memEnd % PAGE_SIZE); // make sure memEnd is on a page boundary.
	nFrames = memEndPage / PAGE_SIZE;

	frames = new BitArray(nFrames);

	// Make a page directory.
	kernelDirectory = new(true/*page align*/) PageDirectory();
	currentDirectory = kernelDirectory;

	// Map some pages in the kernel heap area.
	// Here we call getPage but not allocFrame. This causes PageTables
	// to be created where nessecary. We can't allocate frames yet because
	// they need to be identity mapped first below.
	for (uint32_t i = HEAP_START; i < HEAP_END; i += PAGE_SIZE)
	{
		kernelDirectory->getPage(i, /*make:*/true);
	}

	// Map some pages in the user heap area.
	// Here we call getPage but not allocFrame. This causes PageTables
	// to be created where nessecary. We can't allocate frames yet because
	// they need to be identity mapped first below.
	for (uint32_t i = USER_HEAP_START; i < USER_HEAP_END; i += PAGE_SIZE)
	{
		kernelDirectory->getPage(i, /*make:*/true);
	}

	// Identity map from KERNEL_START to placementAddress.
	uint32_t i = 0;
	while (i < placementAddress)
	{
		// Kernel code is readable but not writable from userspace.
		allocFrame(kernelDirectory->getPage(i, true) , /*kernel:*/false, /*writable:*/false);
		i += PAGE_SIZE;
	}

// TODO BUG HERE!
	// Now allocate those pages we mapped earlier.
	for (i = HEAP_START; i < HEAP_START+HEAP_INITIAL_SIZE; i += PAGE_SIZE)
	{
		// Heap is readable but not writable from userspace.
		allocFrame(kernelDirectory->getPage(i, true), false, false);
	}

	for (i = USER_HEAP_START; i < USER_HEAP_START+USER_HEAP_INITIAL_SIZE; i += PAGE_SIZE)
	{
		allocFrame(kernelDirectory->getPage(i, true), false, true);
	}

	// write the page directory.
	writePageDirectory((address_t)kernelDirectory->getPhysical());
	enablePaging();

	// Initialise the heaps.
	heap.init(HEAP_START, HEAP_START+HEAP_INITIAL_SIZE, HEAP_END & PAGE_MASK /* see memory map */, true);
	userHeap.init(USER_HEAP_START, USER_HEAP_START+USER_HEAP_INITIAL_SIZE, USER_HEAP_END & PAGE_MASK, false);

	heapInitialised = true;
}

void *MemoryManager::malloc(uint32_t size, bool pageAlign, address_t *physicalAddress)
{
	ASSERT(heapInitialised);
	void *addr = heap.allocate(size, pageAlign);
	if (physicalAddress)
	{
		Page *page = kernelDirectory->getPage((address_t)addr, false);
		*physicalAddress = page->frame() + (address_t)addr % PAGE_SIZE;
	}
	return addr;
}

void MemoryManager::free(void *p)
{
	ASSERT(heapInitialised);
	heap.free(p);
}

void *MemoryManager::umalloc(uint32_t size)
{
	ASSERT(heapInitialised);
	return userHeap.allocate(size, false);
}

void MemoryManager::ufree(void *p)
{
	ASSERT(heapInitialised);
	userHeap.free(p);
}

//
// allocFrame -- maps a page to a frame.
//
void MemoryManager::allocFrame(Page *p, bool isKernel, bool isWriteable)
{
	if (p->frame())
	{
		return;
	}
	else
	{
		// TODO: make this more efficient than O(n).
		uint32_t frameIdx = frames->firstClear();
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
// allocFrame -- maps a page to a frame.
//
address_t MemoryManager::allocFrame()
{
	// TODO: make this more efficient than O(n).
	uint32_t frameIdx = frames->firstClear();
	if (frameIdx == (uint32_t)-1)
	{
		PANIC("No free frames.");
	}

	frames->set(frameIdx);

	return frameIdx * PAGE_SIZE;
}

void MemoryManager::freeFrame(Page *p)
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

void MemoryManager::freeFrame(address_t frame)
{
	frames->clear(frame / PAGE_SIZE);
}

extern "C" address_t initialEsp; // in loader.s

void MemoryManager::remapStack()
{
	ASSERT(currentDirectory);

	uint32_t i;
	// Allocate some space for the stack.
	for (i = STACK_START; i > (STACK_START-STACK_INITIAL_SIZE); i -= PAGE_SIZE)
	{
		// General-purpose stack is in user-mode.
		allocFrame(currentDirectory->getPage(i, /*make:*/true), /*kernel:*/false);
	}

	// Flush the TLB
	flushPageDirectory();

	address_t oldStackPointer = readStackPointer();
	address_t oldBasePointer  = readBasePointer();
	size_t stackSize = initialEsp - oldStackPointer;

	int offset = STACK_START - initialEsp;

	address_t newStackPointer = oldStackPointer + offset;
	address_t newBasePointer = oldBasePointer + offset;

	kconsole.print("Remapping stack from %p-%p to %p-%p (%d bytes)..", initialEsp, oldStackPointer, STACK_START, newStackPointer, stackSize);

	kernel::copy_memory((void*)newStackPointer, (const void*)oldStackPointer, stackSize);

	writeStackPointer(newStackPointer);
	writeBasePointer(newBasePointer);
	kconsole.print("done\n");
}

void MemoryManager::alignPlacementAddress()
{
	if (placementAddress % PAGE_SIZE) // if it needs aligning at all!
	{
		placementAddress += PAGE_SIZE - placementAddress % PAGE_SIZE;
	}
}

void MemoryManager::allocateRange(address_t startAddress, address_t size)
{
	UNUSED(startAddress);
	UNUSED(size);
/*	Address endAddress = startAddress + size;
	processManager->getProcess()->memoryMap.map(startAddress, endAddress - startAddress, MemoryMap::Local);
	startAddress &= PAGE_MASK;
	endAddress   &= PAGE_MASK;

	for (Address i = startAddress; i <= endAddress; i += PAGE_SIZE)
	{
		allocFrame( currentDirectory->getPage(i, true), false );
	}
	flushPageDirectory();*/
}

uint32_t MemoryManager::getKernelHeapSize()
{
	return heap.getSize();
}

uint32_t MemoryManager::getUserHeapSize()
{
	return userHeap.getSize();
}

void MemoryManager::checkIntegrity()
{
	if(heapInitialised)
	{
		heap.checkIntegrity();
		userHeap.checkIntegrity();
	}
}
