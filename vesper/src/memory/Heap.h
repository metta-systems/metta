//
// Copyright 2007 - 2008, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "OrderedArray.h"
#include "Lockable.h"

#define HEAP_INDEX_SIZE   0x20000

/**
 * Implements a heap. The algorithm is based on dlmalloc and uses tagged
 * areas and an index of free areas.
 * Every free or allocated area (block) has a header and footer around it.
 * The footer has a pointer to the header, with the header also containing
 * size information.
 */
class Heap : public lockable_t
{
public:
	inline Heap() : lockable_t() {}

	/**
	 * Create a new Heap, with start address start, initial size end-start,
	 * and expanding up to a maximum address of max.
	 */
	inline Heap(address_t start, address_t end, address_t max, bool isKernel) { init(start, end, max, isKernel); }

	~Heap();

	void init(address_t start, address_t end, address_t max, bool isKernel);

	/**
	 * Allocates a contiguous region of memory 'size' in size. If pageAlign,
	 * it creates that block starting on a page boundary.
	 */
	void *allocate(uint32_t size, bool pageAlign);

	/**
	 * Releases a block allocated with @c allocate.
	 */
	void free(void *p);

	/**
	 * Tries to detect buffer overruns by walking the heap and checking magic numbers.
	 */
	void checkIntegrity();

	/**
	 * Returns the current heap size. For analysis purposes.
	 */
	inline uint32_t getSize()
	{
		return endAddress - startAddress;
	}

private:
	/**
	 * Increases the size of the heap, by requesting pages to be allocated.
	 * Heap size increases from size to the nearest page boundary after newSize.
	 */
	void expand(uint32_t newSize);

	/**
	 * Decreases the size of the heap, by requesting pages to be deallocated.
	 * Heap size decreases from size to the nearest page boundary after newSize.
	 * Returns the new size (endAddress - startAddress) - not guaranteed to be the
	 * same as newSize.
	 */
	uint32_t contract(uint32_t newSize);

	/**
	 * Find smallest place suitable for allocation.
	 */
	int32_t findSmallestHole(uint32_t size, bool pageAlign);

private:
	/**
	 * Size information for a hole/block
	 */
	struct Header
	{
// 		const uint32_t NBACKTRACE = 12;

		uint32_t magic; // Magic number, used for error checking and identification.
		bool    isHole; // true if this is a hole. false if this is a block.
		uint32_t size;  // size of the block, including the end footer.
// 		uint32_t pid;   // owner PID (unused FIXME: remove)
// 		uint32_t backtrace[NBACKTRACE];

		inline int operator < (const Header &b)
		{
			return size < b.size;
		}
	};

	struct Footer
	{
		uint32_t magic;  // Magic number, same as in Header.
		Header*  header; // Pointer to the block header.
	};

	/**
	 * The index table - lists all available holes.
	 */
	OrderedArray<Header, HEAP_INDEX_SIZE>* index;
	/**
	 * The start of our allocated space. Includes index table.
	 */
	uint32_t startAddress;
	/**
	 * The end of our currently allocated space. May be expanded up to maxAddress.
	 */
	uint32_t endAddress;
	/**
	 * The maximum possible address our heap can be expanded to.
	 */
	uint32_t maxAddress;
	/**
	 * If any pages requested by us should be marked as supervisor-only.
	 */
	bool isKernel;
};

// kate: indent-width 4; replace-tabs on;
// vi:set ts=4:set expandtab=on:
