//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2010, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "memory_v1_interface.h"
#include "heap_v1_interface.h"
#include "lockable.h"

//At least sizeof(heap_t)+3*sizeof(heap_t::heap_rec_t)
#define HEAP_MIN_SIZE (256)

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
    inline heap_t(address_t start, address_t end)//, heap_v1_closure* heap_closure)
        : lockable_t()
    {
        init(start, end);//, heap_closure);
    }

    void init(address_t start, address_t end);//, heap_v1_closure* heap_closure);

    /*!
     * Allocates a contiguous region of memory @a size in size.
     * @return start address of the allocated memory block.
     */
    void* allocate(size_t size);

    /*!
     * Releases a block allocated with @a allocate.
     * Releasing a NULL pointer is safe and has no effect.
     */
    void free(void* p);

    /*!
     * Reallocate memory block starting at @a ptr to be of size @a size.
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

    int find_index(size_t size);

private:
    struct heap_rec_t
    {
        memory_v1_size prev;  // either a magic or size of previous block (backlink).
        memory_v1_size size;  // size of allocated block, including the end footer.
        int32_t        index; // allocation table index.
        union {
            heap_t/*v1_closure*/* heap; // when busy
            heap_rec_t*      next; // when free
        };
    };

    static heap_rec_t* prev_block(heap_rec_t* rec);
    static heap_rec_t* next_block(heap_rec_t* rec);
    heap_rec_t* get_new_block(size_t size, int index);
    heap_rec_t* get_new_block_internal(size_t size, int index);

    void coalesce();
    void coalesce_merge_blocks(int32_t index);
    void coalesce_move_blocks(int32_t index);

    static const int SMALL_BLOCKS = 16;
    static const int LARGE_BLOCKS = 24;
    static const int COUNT = (SMALL_BLOCKS + LARGE_BLOCKS + 1);
    static const memory_v1_size all_sizes[COUNT];

    heap_rec_t* blocks[COUNT];
    heap_rec_t* null_malloc;

    /*!
     * The start of our allocated space.
     */
    address_t start_address;
    /*!
     * The end of our currently allocated space. May be expanded if heap type supports it.
     */
    address_t end_address;
};

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
