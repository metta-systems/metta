//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2010, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "ordered_array.h"
#include "lockable.h"

#define HEAP_INDEX_SIZE   0x20000 // 131072

/*!
* Implements a heap. The algorithm is based on dlmalloc and uses tagged
* areas and an index of free areas.
* Every free or allocated area (block) has a header and footer around it.
* The footer has a pointer to the header, with the header also containing
* size information.
*/
class heap_t : public lockable_t
{
public:
    inline heap_t() : lockable_t() {}

    /*!
    * Create a new Heap, with start address @a start, initial size @a end minus @a start,
    * and expanding up to a maximum address of @a max.
    */
    inline heap_t(address_t start, address_t end, address_t max, bool is_kernel)
        : lockable_t()
    {
        init(start, end, max, is_kernel);
    }

    void init(address_t start, address_t end, address_t max, bool is_kernel);

    /*!
    * Allocates a contiguous region of memory @a size in size. If @a page_align is @a true,
    * it creates that block starting on a page boundary.
    * @return start address of the allocated memory block.
    */
    void* allocate(size_t size, bool page_align);

    /*!
    * Releases a block allocated with @a allocate.
    * Releasing a NULL pointer is safe and has no effect.
    */
    void free(void* p);

    /*!
    * Reallocate memory block starting at @a ptr to be of size @a size.
    * For general characteristics of the algorithm see memory_manager::realloc.
    * Will inherit page_align property from the previously allocated block.
    * @return start address of the memory block.
    */
    void* realloc(void* ptr, size_t size);

    /*!
    * Tries to detect buffer overruns by walking the heap and checking magic numbers.
    */
    void check_integrity();

    /*!
    * @return the current heap size. For analysis purposes.
    */
    inline size_t size()
    {
        return end_address - start_address;
    }

private:
    /*!
    * Increase the size of the heap, by requesting pages to be allocated.
    * Heap size increases from @a size to the nearest page boundary above @a new_size.
    */
    void expand(size_t new_size);

    /*!
    * Decrease the size of the heap, by requesting pages to be deallocated.
    * Heap size decreases from @a size to the nearest page boundary above @a new_size.
    * @returns the new size (end_address minus start_address) which is not guaranteed to be the
    * same as @a new_size.
    */
    size_t contract(size_t new_size);

    /*!
    * Find smallest place suitable for allocation.
    */
    int32_t find_smallest_hole(size_t size, bool page_align);

private:
    /*!
    * Size information for a hole/block
    */
    struct header_t
    {
        uint32_t magic;   ///< Magic number, used for error checking and identification.
        bool     is_hole; ///< @a true if this is a hole. @a false if this is a block.
        size_t   size;    ///< Size of the block, including the end footer.

        inline int operator < (const header_t& b)
        {
            return size < b.size;
        }
    };

    struct footer_t
    {
        uint32_t   magic;  ///< Magic number, same as in header.
        header_t*  header; ///< Pointer to the block header.
    };

    /*!
    * The index table - lists all available holes.
    */
    ordered_array_t<header_t, HEAP_INDEX_SIZE>* index;
    /*!
    * The start of our allocated space. Includes index table.
    */
    address_t start_address;
    /*!
    * The end of our currently allocated space. May be expanded up to max_address.
    */
    address_t end_address;
    /*!
    * The maximum possible address our heap can be expanded to.
    */
    address_t max_address;
    /*!
    * If any pages requested by us should be marked as supervisor-only.
    */
    bool is_kernel;
};

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
