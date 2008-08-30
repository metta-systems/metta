#include "Heap.h"
#include "Globals.h"
#include "MemoryManager.h"
#include "Registers.h" // criticalSection() (TODO: move decls to Globals.h)

#define HEAP_MAGIC        0x123890AB
#define HEAP_MIN_SIZE     0x70000

void Heap::init(Address start, Address end, Address max, bool supervisor)
{
	startAddress = start;
	endAddress = end;
	maxAddress = max;
	isKernel = supervisor;

	kconsole.print("Initializing heap (%08x..%08x, kernel: %d).\n", start, end, isKernel);

	ASSERT(startAddress % PAGE_SIZE == 0);
	ASSERT(endAddress   % PAGE_SIZE == 0);

	// Initialise the index.
	index = (OrderedArray<Header, HEAP_INDEX_SIZE>*)startAddress;

	// Shift the start address to resemble where we can start putting data.
	startAddress += sizeof(*index);

	// make sure startAddress is page-aligned.
	if (startAddress % PAGE_SIZE)
	{
		startAddress += PAGE_SIZE - startAddress % PAGE_SIZE;
	}

	// Start by having just one big hole.
	Header *holeHeader = (Header *)startAddress;
	holeHeader->size = endAddress - startAddress;
	holeHeader->magic = HEAP_MAGIC;
	holeHeader->isHole = true;
	Footer *holeFooter = (Footer *)(startAddress + holeHeader->size - sizeof(Footer));
	holeFooter->header = holeHeader;
	holeFooter->magic = HEAP_MAGIC;

	index->insert(holeHeader);
}

Heap::~Heap()
{
}

// Find the smallest hole that will fit.
int32_t Heap::findSmallestHole(uint32_t size, bool pageAlign)
{
	uint32_t iterator = 0;
	while (iterator < index->count())
	{
		// if the user has requested the memory be page-aligned
		if (pageAlign)
		{
			// page-align the starting point of this header.
			uint32_t location = (uint32_t)index->lookup(iterator);
			int32_t offset = 0;
			if ((location + sizeof(Header)) % PAGE_SIZE)
			{
				offset = PAGE_SIZE - (location + sizeof(Header)) % PAGE_SIZE;
			}

			int32_t holeSize = index->lookup(iterator)->size;
			holeSize -= offset;

			// can we fit now?
			if (holeSize >= (int32_t)size)
			{
				break;
			}
		}
		else if (index->lookup(iterator)->size >= size)
		{
			break;
		}

		iterator++;
	}

	if (iterator == index->count())
	{
		return -1;
	}
	else
	{
		return iterator;
	}
}

void *Heap::allocate(uint32_t size, bool pageAlign)
{
	criticalSection();
#ifdef HEAP_DEBUG
	checkIntegrity();
#endif

	// Take into account the header/footer.
	uint32_t newSize = size + sizeof(Header) + sizeof(Footer);

	int32_t iterator = findSmallestHole(newSize, pageAlign);

	if (iterator == -1) // If we didn't find a suitable hole
	{
		uint32_t oldLength = endAddress - startAddress;
		uint32_t oldEndAddress = endAddress;

		// Allocate some more space.
		expand(oldLength + newSize);

		uint32_t newLength = endAddress - startAddress;

		// Find the endmost header. (Not endmost in size, endmost in location)
		uint32_t iterator2 = 0;
		int32_t idx = -1;
		uint32_t value = 0x0;

		while (iterator2 < index->count())
		{
			if ((uint32_t)index->lookup(iterator2) > value)
			{
				value = (uint32_t)index->lookup(iterator2);
				idx = iterator2;
			}
			iterator2++;
		}

		// If we didnt find ANY headers, we need to add one.
		if (idx == -1)
		{
			Header *header = (Header *)oldEndAddress;
			header->magic = HEAP_MAGIC;
			header->size = newLength - oldLength;
			header->isHole = true;
			Footer *footer = (Footer *)((uint32_t)header + header->size - sizeof(Footer));
			footer->magic = HEAP_MAGIC;
			footer->header = header;
			// Put this new header in the index
			index->insert(header);
		}
		else
		{
			// The last header is the one whose size needs adjusting.
			Header *header = index->lookup(idx);
			header->size += newLength - oldLength;
			// Rewrite it's footer.
			Footer *footer = (Footer *)((uint32_t)header + header->size - sizeof(Footer));
			footer->header = header;
			footer->magic = HEAP_MAGIC;
		}
#ifdef HEAP_DEBUG
		checkIntegrity();
#endif
		// TODO: optimize tail-recursion
		endCriticalSection();
		return allocate(size, pageAlign);
	}

	if (index->lookup(iterator)->magic != HEAP_MAGIC)
	{
		kconsole.print("block: %08x\n", index->lookup(iterator));
	}
	ASSERT(index->lookup(iterator)->magic == HEAP_MAGIC);

	uint32_t origHolePos  = (uint32_t)index->lookup(iterator);
	uint32_t origHoleSize = (uint32_t)index->lookup(iterator)->size;

	// If the original hole size minus the requested hole size is less than
	// the space required to make a new hole (sizeof(Header)+sizeof(Footer)),
	// then just use the origHoleSize.
	if (origHoleSize-newSize < sizeof(Header)+sizeof(Footer))
	{
		size += origHoleSize - newSize;
		newSize = origHoleSize;
	}

	// If we need to page-align the data, do it now and make a new hole.
	if (pageAlign && origHolePos % PAGE_SIZE)
	{
		uint32_t newLocation = origHolePos + PAGE_SIZE - origHolePos % PAGE_SIZE - sizeof(Header);
		Header *holeHeader = (Header *)origHolePos;
		holeHeader->size   = PAGE_SIZE - origHolePos % PAGE_SIZE - sizeof(Header);
		holeHeader->magic  = HEAP_MAGIC;
		holeHeader->isHole = true;
		Footer *holeFooter = (Footer *)((uint32_t)newLocation - sizeof(Footer));
		holeFooter->magic  = HEAP_MAGIC;
		holeFooter->header = holeHeader;
		origHolePos        = newLocation;
		origHoleSize       = origHoleSize - holeHeader->size;
	}
	else
	{
		// Delete the hole.
		index->remove(iterator);
	}

	// Overwrite the original header.
	Header *blockHeader  = (Header *)origHolePos;
	blockHeader->magic   = HEAP_MAGIC;
	blockHeader->isHole  = false;
	blockHeader->size    = newSize;
/*	if (processManager->getProcess())
	{
		blockHeader->pid   = processManager->getProcess()->getPid();
		for(int i = 0; i < NBACKTRACE; i++)
		{
		blockHeader->backtrace[i] = Kernel::backtrace(i);
	//       kerr << hex << blockHeader->backtrace[i] << ", ";
		}
	}
	else
	{
		blockHeader->pid   = 0;
		for(int i = 0; i < Header::NBACKTRACE; i++)
		{
			blockHeader->backtrace[i] = 0;
		}
	}*/

	// And the footer...
	Footer *blockFooter  = (Footer *)(origHolePos + sizeof(Header) + size);
	blockFooter->magic   = HEAP_MAGIC;
	blockFooter->header  = blockHeader;

	// If the new hole wouldn't have size zero...
	if (origHoleSize - newSize)
	{
		// Write it.
		Header *holeHeader = (Header *)(origHolePos+sizeof(Header)+size+sizeof(Footer));
		holeHeader->magic  = HEAP_MAGIC;
		holeHeader->isHole = true;
		holeHeader->size   = origHoleSize - newSize;

		Footer *holeFooter = (Footer *)((uint32_t)holeHeader + origHoleSize - newSize - sizeof(Footer));
		if ((Address)holeFooter < HEAP_START || (Address)holeFooter > USER_HEAP_END)
		{
			kconsole.set_color(LIGHTRED);
			kconsole.print("Footer: %p\norigHoleSize: %d\nnewSize: %d\nheader: %p\n", holeFooter, origHoleSize, newSize, holeHeader);
		}
		if ((Address)holeFooter < endAddress)
		{
			holeFooter->magic  = HEAP_MAGIC;
			holeFooter->header = holeHeader;
		}

		// Put the new hole in the index.
		index->insert(holeHeader);
	}

#ifdef HEAP_DEBUG
	checkIntegrity();
#endif
	endCriticalSection();

	return (void *)((uint32_t)blockHeader + sizeof(Header));
}

void Heap::free(void *p)
{
	criticalSection();
#ifdef HEAP_DEBUG
	checkIntegrity();
#endif

	// Exit gracefully for null pointers.
	if (!p)
	{
		return;
	}

	// Get the header and footer associated with this pointer.
	Header *header = (Header *)((uint32_t)p - sizeof(Header));
	Footer *footer = (Footer *)((uint32_t)header + header->size - sizeof(Footer));

	// Consistency check...
	if (header->magic != HEAP_MAGIC)
	{
		kconsole.set_color(LIGHTRED);
		kconsole.print("Header: %p, magic %08x, size: %d\n", header, header->magic, header->size);
	}
	ASSERT(header->magic == HEAP_MAGIC);
	ASSERT(footer->magic == HEAP_MAGIC);
	ASSERT(!header->isHole);

	// Make us a hole.
	header->isHole = true;

	// Do we want to add the header into the index?
	bool doAdd = true;

	// Unify left
	// if the thing immediately to the left of us is a hole footer...
	Footer *testFooter = (Footer *) ( (uint32_t)header - sizeof(Footer) );
	if (testFooter->magic == HEAP_MAGIC && testFooter->header->isHole)
	{
		// cache our current size.
		uint32_t cacheSize = header->size;

		// rewrite our header with the new one
		header = testFooter->header;

		// rewrite our footer to point to the new header.
		footer->header = header;

		// change the size.
		header->size += cacheSize;

		// Since this header is already in the index, we don't want to add it again.
		doAdd = false; // FIXME: sorting by size may get skewed?
	}

	// Unify right
	// if the the thing immediately to the right of us is a hole header...
	Header *testHeader = (Header *)((uint32_t)footer + sizeof(Footer));
	if (testHeader->magic == HEAP_MAGIC && testHeader->isHole)
	{
		// increase our size.
		header->size += testHeader->size;

		// rewrite it's footer to point to our header.
		testFooter = (Footer *)((uint32_t)testHeader + testHeader->size - sizeof(Footer));
		testFooter->header = header;

		// It's now OUR footer! muahahaha....
		footer = testFooter;

		// find and remove this header from the index.
		uint32_t iterator = 0;
		while ((iterator < index->count()) && (index->lookup(iterator) != testHeader))
		{
			iterator ++;
		}

		// Make sure we actually found the item.
		ASSERT(iterator < index->count());

		// Remove it.
		index->remove(iterator);
	}

	// If the footer location is the end address, we can contract.
	if ((uint32_t)footer + sizeof(Footer) == endAddress)
	{
		uint32_t oldLength = endAddress - startAddress;
		uint32_t newLength = contract((uint32_t)header - startAddress);

		// Check how big we will be after resizing
		if (header->size - (oldLength - newLength) > 0)
		{
			// we still exist, resize us.
			header->size -= oldLength - newLength;
			footer = (Footer *)((uint32_t)header + header->size - sizeof(Footer));
			footer->magic = HEAP_MAGIC;
			footer->header = header;
		}
		else
		{
			// We no longer exist :(. Remove us from the index.
			uint32_t iterator = 0;
			while ((iterator < index->count()) && (index->lookup(iterator) != header))
			{
				iterator ++;
			}

			// If we didnt find ourselves, we have nothing to remove.
			if (iterator < index->count())
			{
				index->remove(iterator);
			}

			// ...and nothing to add.
			doAdd = false;
		}
#ifdef HEAP_DEBUG
		checkIntegrity();
#endif
	}

	// Add us to the index
	if (doAdd)
	{
		index->insert(header);
	}

#ifdef HEAP_DEBUG
	checkIntegrity();
#endif
	endCriticalSection();
}

void Heap::expand(uint32_t newSize)
{
#ifdef HEAP_DEBUG
	checkIntegrity();
#endif
	// Sanity check.
	ASSERT(newSize > endAddress - startAddress);

	kconsole.print("Heap expanding from %d to %d\n", endAddress - startAddress, newSize);

	// Get the nearest following page boundary.
	if (newSize % PAGE_SIZE)
	{
		newSize &= PAGE_MASK;
		newSize += PAGE_SIZE;
	}

	// Make sure we are not overreaching ourselves.
	ASSERT(startAddress + newSize <= maxAddress);

	// This should always be on a page boundary.
	uint32_t oldSize = endAddress - startAddress;

	uint32_t i = oldSize;
	while(i < newSize)
	{
		memoryManager.allocFrame(memoryManager.getKernelDirectory()->getPage(startAddress+i), isKernel);
		i += PAGE_SIZE;
	}

	endAddress = startAddress + newSize;
#ifdef HEAP_DEBUG
	checkIntegrity();
#endif
}

uint32_t Heap::contract(uint32_t newSize)
{
#ifdef HEAP_DEBUG
	checkIntegrity();
#endif
	// Sanity check.
	ASSERT(newSize < endAddress - startAddress);

	// get the nearest following page boundary.
	if (newSize % PAGE_SIZE)
	{
		newSize += PAGE_SIZE - newSize % PAGE_SIZE;
	}

	// Don't contract too far.
	if (newSize < HEAP_MIN_SIZE)
		newSize = HEAP_MIN_SIZE;

	kconsole.print("Heap contracting from %d to %d\n", endAddress - startAddress, newSize);

	// Make sure we are not overreaching ourselves.
	ASSERT(newSize > 0);

	uint32_t oldSize = endAddress-startAddress;

	uint32_t i = newSize;
	while(i < oldSize)
	{
		memoryManager.freeFrame(memoryManager.getKernelDirectory()->getPage(startAddress+i));
		i += PAGE_SIZE;
	}

	endAddress = startAddress+newSize;
#ifdef HEAP_DEBUG
	checkIntegrity();
#endif
	return newSize;
}

void Heap::checkIntegrity()
{
#ifdef HEAP_DEBUG
	// We should, by starting at startAddress, be able to walk through all blocks/
	// holes and check their magic numbers.
	uint32_t addr = startAddress;
	Header *lastHeader = NULL;
	Header *thisHeader = (Header*)startAddress;
	Header *nextHeader = (Header*)((uint32_t)thisHeader + thisHeader->size);
	if ((uint32_t)nextHeader >= endAddress)
		nextHeader = NULL;

	while (thisHeader)
	{
		if (thisHeader->magic != HEAP_MAGIC)
		{
			// header overwritten.
			kconsole.set_color(LIGHTRED);
			kconsole.print(
			"\nPrevious block:\n  Address: %p\n  Size: %d\n  Hole: %d\n"
			"This block:\n  Address: %p\n  Size: %d\n  Hole: %d\n",
			lastHeader, lastHeader->size, lastHeader->isHole,
			thisHeader, thisHeader->size, thisHeader->isHole);
			PANIC("Heap header overwritten!");
		}

		if (!nextHeader)
			break;

		Footer *footer = (Footer*)((uint32_t)nextHeader - sizeof(Footer));
		if (footer->magic != HEAP_MAGIC)
		{
			// footer overwritten.
			kconsole.set_color(LIGHTRED);
			kconsole.print(
			"\nPrevious block:\n  Address: %p\n  Size: %d\n  Hole: %d\n"
			"This block:\n  Address: %p\n  Size: %d\n  Hole: %d\n",
			"Next block:\n  Address: %p\n  Size: %d\n  Hole: %d\n",
			lastHeader, lastHeader->size, lastHeader->isHole,
			thisHeader, thisHeader->size, thisHeader->isHole);
			nextHeader, nextHeader->size, nextHeader->isHole);
			PANIC("Heap footer overwritten!");
		}

		lastHeader = thisHeader;
		thisHeader = nextHeader;
		nextHeader = (Header*)((uint32_t)thisHeader + thisHeader->size);
		if ((uint32_t)nextHeader >= endAddress)
			nextHeader = NULL;
	}
#endif
}
