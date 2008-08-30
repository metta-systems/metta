#include "MemoryManager.h"
// #include <MonitorDriver.h>
// #include <Asm.h>
// #include <Kernel.h>
// #include <ProcessManager.h>

extern Address end; // defined by linker.ld
// extern Address initialEsp;

MemoryManager::MemoryManagement()
{
	placementAddress = (Address)&end ; // TODO: change to multiboot->mod_end
	heapInitialised = false;
	currentDirectory = kernelDirectory = NULL;
}

MemoryManager::~MemoryManagement()
{
}

void MemoryManager::init(Address memEnd)
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
	for (int i = HEAP_START; i < HEAP_END; i += PAGE_SIZE)
	{
		kernelDirectory->getPage(i, /*make:*/true);
	}

	// Map some pages in the user heap area.
	// Here we call getPage but not allocFrame. This causes PageTables
	// to be created where nessecary. We can't allocate frames yet because
	// they need to be identity mapped first below.
	for (int i = USER_HEAP_START; i < USER_HEAP_END; i += PAGE_SIZE)
	{
		kernelDirectory->getPage(i, /*make:*/true);
	}

	// Identity map from KERNEL_START to placementAddress.
	int i = 0;
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
	writePageDirectory((Address)kernelDirectory->getPhysical());
	enablePaging();

	// Initialise the heaps.
	heap = Heap(HEAP_START, HEAP_START+HEAP_INITIAL_SIZE, HEAP_END & PAGE_MASK /* see memory map */);
	userHeap = Heap(USER_HEAP_START, USER_HEAP_START+USER_HEAP_INITIAL_SIZE, USER_HEAP_END & PAGE_MASK);

	heapInitialised = true;
}

void *MemoryManager::malloc(uint32_t size, bool pageAlign, Address *physicalAddress)
{
	ASSERT(heapInitialised);
	void *addr = heap.allocate(size, pageAlign);
	if (physicalAddress)
	{
		Page *page = kernelDirectory->getPage((Address)addr, false);
		*physicalAddress = page->frame() + (Address)addr % PAGE_SIZE;
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
Address MemoryManager::allocFrame()
{
	// TODO: make this more efficient than O(n).
	uint32_t frameIdx = frames->firstClear();
	if (frameIdx == (u32int)-1)
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

void MemoryManager::freeFrame(Address frame)
{
	frames->clear(frame / PAGE_SIZE);
}

void MemoryManager::remapStack()
{
	ASSERT(currentDirectory);

	uint32_t i;
	// Allocate some space for the stack.
	for (i = STACK_START; i > (STACK_START-STACK_INITIAL_SIZE); i -= PAGE_SIZE)
	{
		// General-purpose stack is in user-mode.
		allocFrame(currentDirectory->getPage(i, /*make:*/true), /*kenrel:*/false);
	}

	// Flush the TLB
	flushPageDirectory();

	Address oldStackPointer = readStackPointer();
	Address oldBasePointer  = readBasePointer();
	Address offset          = STACK_START - initialEsp;
	Address newStackPointer = oldStackPointer + offset;
	Address newBasePointer  = oldBasePointer  + offset;

	// Copy the stack.
	Kernel::memcpy(newStackPointer, oldStackPointer, initialEsp - oldStackPointer);

  // Backtrace through the original stack, copying new values into
  // the new stack.
  Address origBasePtrAddr = oldBasePointer;
  Address origBasePtrVal;

  for(u32int i = STACK_START; i > STACK_START-STACK_INITIAL_SIZE; i -= 4)
  {
    Address tmp = * (Address*)i;
    if (( oldStackPointer < tmp) && (tmp < initialEsp))
    {
      tmp = tmp + offset;
      Address *tmp2 = (Address*)i;
      *tmp2 = tmp;
    }
  }

  writeStackPointer(newStackPointer);
  writeBasePointer(newBasePointer);
}

void MemoryManager::alignPlacementAddress()
{
	if (placementAddress % PAGE_SIZE) // if it needs aligning at all!
	{
		placementAddress += PAGE_SIZE - placementAddress % PAGE_SIZE;
	}
}

void MemoryManager::allocateRange(Address startAddress, Address size)
{
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
