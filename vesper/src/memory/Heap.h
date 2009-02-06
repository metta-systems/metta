//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "ordered_array.h"
#include "lockable.h"

using metta::common::ordered_array;

namespace metta {
namespace kernel {

#define HEAP_INDEX_SIZE   0x20000

/**
* Implements a heap. The algorithm is based on dlmalloc and uses tagged
* areas and an index of free areas.
* Every free or allocated area (block) has a header and footer around it.
* The footer has a pointer to the header, with the header also containing
* size information.
**/
class heap : public lockable
{
public:
    inline heap() : lockable() {}

    /**
    * Create a new Heap, with start address start, initial size end-start,
    * and expanding up to a maximum address of max.
    **/
    inline heap(address_t start, address_t end, address_t max, bool is_kernel)
        : lockable()
    {
        init(start, end, max, is_kernel);
    }

//     ~heap();

    void init(address_t start, address_t end, address_t max, bool is_kernel);

    /**
    * Allocates a contiguous region of memory @p size in size. If @p pageAlign,
    * it creates that block starting on a page boundary.
    **/
    void *allocate(size_t size, bool page_align);

    /**
    * Releases a block allocated with @c allocate.
    **/
    void free(void *p);

    /**
    * Reallocate memory block starting at ptr to be of size @c size.
    * For general characteristics of the algorithm see memory_manager::realloc.
    * Will inherit page_align property from the previously allocated block.
    **/
    void *realloc(void *ptr, size_t size);

    /**
    * Tries to detect buffer overruns by walking the heap and checking magic numbers.
    **/
    void check_integrity();

    /**
    * Returns the current heap size. For analysis purposes.
    **/
    inline size_t size()
    {
        return end_address - start_address;
    }

private:
    /**
    * Increases the size of the heap, by requesting pages to be allocated.
    * Heap size increases from @c size to the nearest page boundary after @p new_size.
    **/
    void expand(size_t new_size);

    /**
    * Decreases the size of the heap, by requesting pages to be deallocated.
    * Heap size decreases from @c size to the nearest page boundary after @p new_size.
    * Returns the new size (endAddress - startAddress) - not guaranteed to be the
    * same as @p new_size.
    */
    size_t contract(size_t new_size);

    /**
    * Find smallest place suitable for allocation.
    **/
    int32_t find_smallest_hole(size_t size, bool page_align);

private:
    /**
    * Size information for a hole/block
    **/
    struct header
    {
        uint32_t magic;   ///< Magic number, used for error checking and identification.
        bool     is_hole; ///< @c true if this is a hole. @c false if this is a block.
        size_t   size;    ///< Size of the block, including the end footer.

        inline int operator < (const header &b)
        {
            return size < b.size;
        }
    };

    struct footer
    {
        uint32_t        magic;  ///< Magic number, same as in header.
        struct header*  header; ///< Pointer to the block header.
    };

    /**
    * The index table - lists all available holes.
    **/
    ordered_array<header, HEAP_INDEX_SIZE>* index;
    /**
    * The start of our allocated space. Includes index table.
    **/
    address_t start_address;
    /**
    * The end of our currently allocated space. May be expanded up to max_address.
    **/
    address_t end_address;
    /**
    * The maximum possible address our heap can be expanded to.
    **/
    address_t max_address;
    /**
    * If any pages requested by us should be marked as supervisor-only.
    **/
    bool is_kernel;
};

}
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
