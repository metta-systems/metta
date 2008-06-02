#include "kalloc.h"
#include "paging.h"

#define HEAP_INDEX_SIZE   0x20000
#define HEAP_MAGIC        0x123890AB
#define HEAP_MIN_SIZE     0x70000

// end is defined in the linker script.
extern uint32_t end;
uint32_t placement_address = (uint32_t)&end;
DefaultHeap *kheap = 0;

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
	if (kheap)
	{
		void *ptr = kheap->alloc(sz, align);
		if (phys)
			*phys = (uint32_t)ptr;
		return (uint32_t)ptr;
	}

	// If no kernel heap exists, we just assign memory at placement_address
	// and increment it by sz.
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

void kfree(uint32_t ptr)
{
	if (kheap)
		kheap->free((void *)ptr);
}

/** Heap allocation */
int32_t DefaultHeap::find_smallest_hole(uint32_t size, uint8_t page_align)
{
	// Find the smallest hole that will fit.
	uint32_t iterator = 0;
	while (iterator < index.size)
	{
		HeapHeader *header = (HeapHeader *)index.lookup(iterator);
		// If the user has requested the memory be page-aligned
		if (page_align > 0)
		{
			// Page-align the starting point of this header.
			uint32_t location = (uint32_t)header;
			int32_t offset = 0;
			if (((location+sizeof(HeapHeader)) & 0xFFFFF000) != 0)
				offset = 0x1000 /* page size */  - (location+sizeof(HeapHeader))%0x1000;
			int32_t hole_size = (int32_t)header->size - offset;
			// Can we fit now?
			if (hole_size >= (int32_t)size)
				break;
		}
		else
		if (header->size >= size)
			break;
		iterator++;
	}
	// Why did the loop exit?
	if (iterator == index.size)
		return -1; // We got to the end and didn't find anything.
	else
		return iterator;
}

void DefaultHeap::expand(uint32_t new_size)
{
	// Sanity check.
	ASSERT(new_size > end_address - start_address);

	// Get the nearest following page boundary.
	if ((new_size & 0xFFFFF000) != 0) ///WTF?
	{
		new_size &= 0xFFFFF000;
		new_size += 0x1000;
	}

	// Make sure we are not overreaching ourselves.
	ASSERT(start_address+new_size <= max_address);

	// This should always be on a page boundary.
	uint32_t old_size = end_address - start_address;
	uint32_t i = old_size;
	while (i < new_size)
	{
	///!!!
		alloc_frame( paging.get_page(start_address+i, 1, kernel_directory),
					(supervisor)?1:0, (readonly)?0:1);
		i += 0x1000 /* page size */;
	}
	end_address = start_address + new_size;
}

uint32_t DefaultHeap::contract(uint32_t new_size)
{
	// Sanity check.
	ASSERT(new_size < end_address - start_address);

	// Get the nearest following page boundary.
	if (new_size & 0x1000)
	{
		new_size &= 0x1000;
		new_size += 0x1000;
	}

	// Don't contract too far!
	if (new_size < HEAP_MIN_SIZE)
		new_size = HEAP_MIN_SIZE;

	uint32_t old_size = end_address - start_address;
	uint32_t i = old_size - 0x1000;
	while (new_size < i)
	{
		free_frame(paging.get_page(start_address+i, 0, kernel_directory));
		i -= 0x1000;
	}
	end_address = start_address + new_size;
	return new_size;
}

DefaultHeap::DefaultHeap(uint32_t start, uint32_t end, uint32_t max, bool supervisor, bool readonly)
	: index((void *)(start), HEAP_INDEX_SIZE)
{
	// All our assumptions are made on startAddress and endAddress being page-aligned.
	ASSERT(start%0x1000 == 0);
	ASSERT(end%0x1000 == 0);

	// Shift the start address forward to resemble where we can start putting data.
	start += sizeof(HeapHeader*) * HEAP_INDEX_SIZE;

	// Make sure the start address is page-aligned.
	if ((start & 0xFFFFF000) != 0)
	{
		start &= 0xFFFFF000;
		start += 0x1000;
	}
	// Write the start, end and max addresses into the heap structure.
	this->start_address = start;
	this->end_address = end;
	this->max_address = max;
	this->supervisor = supervisor;
	this->readonly = readonly;

	// We start off with one large hole in the index.
	HeapHeader *hole = (HeapHeader *)start;
	hole->size = end - start;
	hole->magic = HEAP_MAGIC;
	hole->is_hole = 1;
	index.insert(hole);
}

void *DefaultHeap::alloc(uint32_t size, bool page_align)
{
	// Make sure we take the size of header/footer into account.
	uint32_t new_size = size + sizeof(HeapHeader) + sizeof(HeapFooter);
	// Find the smallest hole that will fit.
	int32_t iterator = find_smallest_hole(new_size, page_align);

	if (iterator == -1) // If we didn't find a suitable hole
	{
		// Save some previous data.
		uint32_t old_length = end_address - start_address;
		uint32_t old_end_address = end_address;

		// We need to allocate some more space.
		expand(old_length + new_size);
		uint32_t new_length = end_address - start_address;

		// Find the endmost header. (Not endmost in size, but in location).
		iterator = 0;
		// Vars to hold the index of, and value of, the endmost header found so far.
		int32_t idx = -1;
		uint32_t value = 0x0;
		while ((uint32_t)iterator < index.size)
		{
			uint32_t tmp = (uint32_t)index.lookup(iterator);
			if (tmp > value)
			{
				value = tmp;
				idx = iterator;
			}
			iterator++;
		}

		// If we didn't find ANY headers, we need to add one.
		if (idx == -1)
		{
			HeapHeader *header = (HeapHeader *)old_end_address;
			header->magic = HEAP_MAGIC;
			header->size = new_length - old_length;
			header->is_hole = 1;
			HeapFooter *footer = (HeapFooter *) (old_end_address + header->size - sizeof(HeapFooter));
			footer->magic = HEAP_MAGIC;
			footer->header = header;
			index.insert(header);
		}
		else
		{
			// The last header needs adjusting.
			HeapHeader *header = index.lookup(idx);
			header->size += new_length - old_length;
			// Rewrite the footer.
			HeapFooter *footer = (HeapFooter *) ( (uint32_t)header + header->size - sizeof(HeapFooter) );
			footer->magic = HEAP_MAGIC;
			footer->header = header;
		}
		// We now have enough space. Recurse, and call the function again.
		return alloc(size, page_align);
	}

	HeapHeader *orig_hole_header = index.lookup(iterator);
	uint32_t orig_hole_pos = (uint32_t)orig_hole_header;
	uint32_t orig_hole_size = orig_hole_header->size;

	// Here we work out if we should split the hole we found into two parts.
	// Is the original hole size minus requested hole size less than the overhead for adding a new hole?
	if (orig_hole_size - new_size < sizeof(HeapHeader)+sizeof(HeapFooter))
	{
		// Then just increase the requested size to the size of the hole we found.
		size += orig_hole_size - new_size;
		new_size = orig_hole_size;
	}

	// If we need to page-align the data, do it now and make a new hole in front of our block.
	if (page_align && (orig_hole_pos & 0xFFFFF000))
	{
		uint32_t new_location = orig_hole_pos + 0x1000 /* page size */ - (orig_hole_pos&0xFFF) - sizeof(HeapHeader);
		HeapHeader *hole_header = (HeapHeader *)orig_hole_pos;
		hole_header->size     = 0x1000 /* page size */ - (orig_hole_pos&0xFFF) - sizeof(HeapHeader);
		hole_header->magic    = HEAP_MAGIC;
		hole_header->is_hole  = 1;
		HeapFooter *hole_footer = (HeapFooter *) ( (uint32_t)new_location - sizeof(HeapFooter) );
		hole_footer->magic    = HEAP_MAGIC;
		hole_footer->header   = hole_header;
		orig_hole_pos         = new_location;
		orig_hole_size        = orig_hole_size - hole_header->size;
	}
	else
	{
		// Else we don't need this hole any more, delete it from the index.
		index.remove(iterator);
	}

	// Overwrite the original header...
	HeapHeader *block_header  = (HeapHeader *)orig_hole_pos;
	block_header->magic     = HEAP_MAGIC;
	block_header->is_hole   = 0;
	block_header->size      = new_size;
	// ...And the footer
	HeapFooter *block_footer  = (HeapFooter *) (orig_hole_pos + sizeof(HeapHeader) + size);
	block_footer->magic     = HEAP_MAGIC;
	block_footer->header    = block_header;

	// We may need to write a new hole after the allocated block.
	// We do this only if the new hole would have positive size...
	if (orig_hole_size - new_size > 0)
	{
		HeapHeader *hole_header = (HeapHeader *) (orig_hole_pos + sizeof(HeapHeader) + size + sizeof(HeapFooter));
		hole_header->magic    = HEAP_MAGIC;
		hole_header->is_hole  = 1;
		hole_header->size     = orig_hole_size - new_size;
		HeapFooter *hole_footer = (HeapFooter *) ( (uint32_t)hole_header + orig_hole_size - new_size - sizeof(HeapFooter) );
		if ((uint32_t)hole_footer < end_address)
		{
			hole_footer->magic = HEAP_MAGIC;
			hole_footer->header = hole_header;
		}
		// Put the new hole in the index;
		index.insert(hole_header);
	}

	// ...And we're done!
	return (void *)((uint32_t)block_header+sizeof(HeapHeader));
}

void DefaultHeap::free(void *p)
{
	// Exit gracefully for null pointers.
	if (p == 0)
		return;

	// Get the header and footer associated with this pointer.
	HeapHeader *header = (HeapHeader*)((uint32_t)p - sizeof(HeapHeader));
	HeapFooter *footer = (HeapFooter*)((uint32_t)header + header->size - sizeof(HeapFooter));

	// Sanity checks.
	ASSERT(header->magic == HEAP_MAGIC);
	ASSERT(footer->magic == HEAP_MAGIC);

	// Make us a hole.
	header->is_hole = 1;

	// Do we want to add this header into the 'free holes' index?
	char do_add = 1;

	// Unify left
	// If the thing immediately to the left of us is a footer...
	HeapFooter *test_footer = (HeapFooter*)((uint32_t)header - sizeof(HeapFooter));
	if (test_footer->magic == HEAP_MAGIC && test_footer->header->is_hole == 1)
	{
		uint32_t cache_size = header->size; // Cache our current size.
		header = test_footer->header;     // Rewrite our header with the new one.
		footer->header = header;          // Rewrite our footer to point to the new header.
		header->size += cache_size;       // Change the size.
		do_add = 0;                       // Since this header is already in the index, we don't want to add it again.
	}

	// Unify right
	// If the thing immediately to the right of us is a header...
	HeapHeader *test_header = (HeapHeader*) ( (uint32_t)footer + sizeof(HeapFooter) );
	if (test_header->magic == HEAP_MAGIC && test_header->is_hole)
	{
		header->size += test_header->size; // Increase our size.
		test_footer = (HeapFooter*)((uint32_t)test_header + test_header->size - sizeof(HeapFooter)); // Rewrite it's footer to point to our header.
		footer = test_footer;
		// Find and remove this header from the index.
		uint32_t iterator = 0;
		while ((iterator < index.size) && (index.lookup(iterator) != test_header))
			iterator++;

		// Make sure we actually found the item.
		ASSERT(iterator < index.size);
		// Remove it.
		index.remove(iterator);
	}

	// If the footer location is the end address, we can contract.
	if ((uint32_t)footer+sizeof(HeapFooter) == end_address)
	{
		uint32_t old_length = end_address - start_address;
		uint32_t new_length = contract((uint32_t)header - start_address);
		// Check how big we will be after resizing.
		if (header->size - (old_length-new_length) > 0)
		{
			// We will still exist, so resize us.
			header->size -= old_length - new_length;
			footer = (HeapFooter*) ( (uint32_t)header + header->size - sizeof(HeapFooter) );
			footer->magic = HEAP_MAGIC;
			footer->header = header;
		}
		else
		{
			// We will no longer exist :(. Remove us from the index.
			uint32_t iterator = 0;
			while ((iterator < index.size) && (index.lookup(iterator) != (void*)test_header))
				iterator++;
			// If we didn't find ourselves, we have nothing to remove.
			if (iterator < index.size)
				index.remove(iterator);
		}
	}

	// If required, add us to the index.
	if (do_add == 1)
		index.insert(header);
}
