#ifndef __INCLUDED_MEMORYMANAGER_H
#define __INCLUDED_MEMORYMANAGER_H

#include "Types.h"
#include <Heap.h>
#include "BitArray.h"

#define HEAP_START              0xC0000000
#define HEAP_INITIAL_SIZE       0x100000
#define HEAP_END                0xCFFFFFFF
#define USER_HEAP_START         0xD0000000
#define USER_HEAP_INITIAL_SIZE  0x100000
#define USER_HEAP_END           0xDFFFFFFF
#define STACK_START             (0xB0000000-0x4)
#define STACK_INITIAL_SIZE      0x10000
#define STACK_END               0xA0000000
#define KERNEL_START            0x100000

#define STACK_ADDRESS(x)     ((Address)x <= STACK_START && (Address)x > STACK_END)
#define HEAP_ADDRESS(x)      ((Address)x >= HEAP_START  && (Address)x < USER_HEAP_START)
#define USER_HEAP_ADDRESS(x) ((Address)x >= USER_HEAP_START && (Address)x <= USER_HEAP_END)

class Page;
class PageDirectory;

/**
 * Handles all memory related events. Heap startup, allocation, deallocation,
 * virtual memory etc.
 */
class MemoryManager
{
public:
	/**
	 * Heap can call our private allocFrame() functions.
	 */
	friend class Heap;

	/**
	 * Default constructor, called on bootup.
	 */
	MemoryManager();

	~MemoryManager();

	/**
		Normal constructor - passes the address of end of memory. Initialises paging and sets up
		a standard kernel page directory. Enables paging, then maps some pages for the heap.
	**/
	void init(Address memEnd);

	/**
		Allocate "size" bytes, returning the physical address of the segment allocated in
		physicalLocation if physicalLocation != NULL.
	**/
	void* malloc(uint32_t size, bool pageAlign = false, Address* physicalLocation = NULL);

	/**
		Deallocate the memory allocated to p.
	**/
	void free(void* p);

	/**
		Allocate "size" bytes from the *user space heap*.
	**/
	void* umalloc(uint32_t size);

	/**
		Deallocate any memory allocated to p via umalloc.
	**/
	void ufree(void* p);

	/**
		Accessor functions for heapInitialised and placementAddress.
	**/
	bool isHeapInitialised() { return heapInitialised; }
	Address getPlacementAddress() { return placementAddress; }
	void setPlacementAddress(Address a) { placementAddress = a; }

	/**
		Forces the placementAddress variable to be PAGE_SIZE aligned.
	**/
	void alignPlacementAddress();

	/**
		Changes the original stack given by the bootloader to one at
		a virtual memory location defined at compile time.
	**/
	void remapStack();

	/**
		Accessor functions for kernelDirectory, currentDirectory
	**/
	PageDirectory* getKernelDirectory()  { return kernelDirectory; }
	PageDirectory* getCurrentDirectory() { return currentDirectory; }
	void setCurrentDirectory(PageDirectory* p) { currentDirectory = p; }

	/**
		Finds a free frame (swaps out if necessary) and allocates it to p.
	**/
	void allocFrame(Page* p, bool isKernel = true, bool isWriteable = true);

	/**
		Finds a free frame and returns it.
	**/
	Address allocFrame();

	/**
		Removes the frame under p's control and returns it to the pool.
	**/
	void freeFrame(Page* p);

	/**
		Adds the previously allocated frame 'frame' and returns it to the pool.
	**/
	void freeFrame(Address frame);

	/**
		Causes the given range of virtual memory to get allocated physical
		memory.
	**/
	void allocateRange(Address startAddress, uint32_t size);

	/**
		Returns the size of the kernel heap. For analysis purposes.
	**/
	uint32_t getKernelHeapSize();
	uint32_t getUserHeapSize();

	/**
		Checks the kernel and user heap for integrity.
	**/
	void checkIntegrity();

private:
	/**
	 * Array of frames to describe physical memory state.
	 */
	BitArray *frames;

	/**
	 * Total number of physical memory frames.
	 */
	uint32_t nFrames;

	/**
		Has the kernel heap been initialised yet?
	**/
	bool heapInitialised;

	/**
		The kernel heap
	**/
	Heap heap;

	/**
		The user-mode shared heap
	**/
	Heap userHeap;

	/**
		Before the heap is initialised, this holds the next available location
		for 'placement new' to be called on.
	**/
	Address placementAddress;

	/**
		The currently active page directory
	**/
	PageDirectory *currentDirectory;

	/**
		Pointer to the "master" page directory. This holds page table pointers for kernel
		space. All other page directories must match the entries in here to maintain easy
		consistency of kernel-space over memory spaces.
	**/
	PageDirectory *kernelDirectory;
};

#include "MemoryManager-arch.h"

#endif
