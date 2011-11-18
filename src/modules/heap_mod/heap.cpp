//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2011, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "heap.h"
#include "memory.h"
#include "debugger.h"
#include "default_console.h"
#include "panic.h"
//#include "config.h" // for HEAP_DEBUG

#define HEAP_DEBUG 1

/*!
 * @class heap_t
 * XXX be careful not to use memory-allocating kconsole calls inside heap_t
 * as heap methods run with heap locked and asking to malloc from inside heap_t
 * will deadlock.
 */

#define HEAP_MAGIC        0xfa11dead
#define MIN_HEAP_OVERHEAD (sizeof(heap_rec_t)*3)

#define OTHER_INDEX (COUNT-1)

#define WORD_SIZE (sizeof(uint64_t))
static inline size_t BLOCK_ALIGN(size_t _x) { return ((_x)+WORD_SIZE) & -(WORD_SIZE); }
#define _S(_x) (_x * WORD_SIZE)

/* Size of minimum fragment: this should be heaprec + all_sizes[0] */
#define MIN_FRAG (sizeof(heap_rec_t) + _S(1))

#define SMALL_LIMIT _S(16)
#define LARGE_LIMIT _S(1296)
#define SMALL_INDEX(x) ((x-1) / WORD_SIZE)

const memory_v1::size heap_t::all_sizes[heap_t::COUNT] =
{
    _S(1),   _S(2),   _S(3),   _S(4),   _S(5),   _S(6),   _S(7),    _S(8),
    _S(9),   _S(10),  _S(11),  _S(12),  _S(13),  _S(14),  _S(15),   _S(16),
    
    _S(19),  _S(23),  _S(28),  _S(34),  _S(41),  _S(49),  _S(59),   _S(71),
    _S(85),  _S(102), _S(122), _S(146), _S(175), _S(210), _S(252),  _S(302),
    _S(362), _S(434), _S(521), _S(625), _S(750), _S(900), _S(1080), _S(1296),
    
    ~0
};

inline heap_t::heap_rec_t* heap_t::prev_block(heap_rec_t* rec)
{
    return reinterpret_cast<heap_rec_t*>(reinterpret_cast<char*>(rec - 1) - rec->prev);
}

inline heap_t::heap_rec_t* heap_t::next_block(heap_rec_t* rec)
{
    return reinterpret_cast<heap_rec_t*>(reinterpret_cast<char*>(rec + 1) + rec->size);
}

void heap_t::init(address_t start, address_t end)//, heap_v1_closure* heap_closure)
{
    start_address = start;
    end_address   = end;

    kconsole << "Initializing heap (" << start << ".." << end << ")." << endl;

    for (int i = 0; i < COUNT; ++i)
        blocks[i] = NULL;

    // First entry is null_malloc marker.
    heap_rec_t* null_m = reinterpret_cast<heap_rec_t*>(start);
    null_m->prev = HEAP_MAGIC;
    null_m->size = 0;
    null_m->index = -1;
    null_m->heap = this;// heap_closure;

    null_malloc = null_m;

    // Second entry is all free space in the heap.
    heap_rec_t* rec = null_m + 1;
    rec->prev = HEAP_MAGIC;
    rec->size = (end - start) - MIN_HEAP_OVERHEAD;
    rec->index = OTHER_INDEX;
    rec->next = NULL;
    
    blocks[OTHER_INDEX] = rec;
    
    // Third entry is end marker.
    heap_rec_t* end_rec = next_block(rec);
    end_rec->prev = rec->size;
    end_rec->size = 0;
    end_rec->index = 0;
    
    ASSERT(reinterpret_cast<char*>(end_rec) == reinterpret_cast<char*>(end_address) - sizeof(heap_rec_t));
    ASSERT(prev_block(end_rec) == rec);
}

// heap_t::~heap_t()
// {
    //TODO: check that everything was deallocated prior to destroying the heap_t
    //Print leak summary.
// }

int heap_t::find_index(size_t size)
{
    if (size <= SMALL_LIMIT)
    {
        return SMALL_INDEX(size);
    }
    else if (size <= LARGE_LIMIT)
    {
        int mi;
        int lo = SMALL_BLOCKS;
        int hi = OTHER_INDEX-1;

        for (; lo + 4 < hi;)
        {
            mi = (lo + hi) / 2;
            if (size == all_sizes[mi])
            {
                return mi;
            }
            else if (size < all_sizes[mi])
                hi = mi;
            else
                lo = mi;
        }
        for(mi = lo; mi <= hi; mi++)
        {
            if (size <= all_sizes[mi])
            {
                return mi;
            }
        }
    }
    return OTHER_INDEX;
}

void heap_t::coalesce()
{
    int32_t index;

    for (index = 0; index <= OTHER_INDEX; ++index)
        coalesce_merge_blocks(index);

    for (index = 0; index <= OTHER_INDEX; ++index)
        coalesce_move_blocks(index);
}

void heap_t::coalesce_merge_blocks(int32_t index)
{
    heap_rec_t* before_block;
    heap_rec_t* after_block;
    heap_rec_t* free_block;
    heap_rec_t** ptr;

    for (ptr = &blocks[index]; (free_block = *ptr); )
    {
        if (free_block->prev != HEAP_MAGIC) // previous block is unallocated indeed!
        {
            before_block = prev_block(free_block);
            if (next_block(before_block) != free_block)
                PANIC("Out of sanity!");
                
            before_block->size += free_block->size + sizeof(heap_rec_t);
            after_block = next_block(free_block);
            after_block->prev = before_block->size;
            
            *ptr = free_block->next;
        }
        else
        {
            ptr = &(free_block->next);
        }
    }
}

void heap_t::coalesce_move_blocks(int32_t index)
{
    heap_rec_t* free_block;
    heap_rec_t** ptr;

    for (ptr = &blocks[index]; (free_block = *ptr); )
    {
        int new_index = find_index(free_block->size);
        if (new_index != free_block->index)
        {
            *ptr = free_block->next;
            free_block->next = blocks[new_index];
            blocks[new_index] = free_block;
            free_block->index = new_index;
        }
        else
        {
            ptr = &(free_block->next);
        }
    }
}

heap_t::heap_rec_t* heap_t::get_new_block_internal(size_t size, int index)
{
    heap_rec_t* free_block;
    heap_rec_t* allocated_block;
    heap_rec_t** ptr;

    for (ptr = &blocks[OTHER_INDEX]; (free_block = *ptr); ptr = &(free_block->next))
    {
        if (free_block->size >= size)
        {
            if (free_block->size - size >= MIN_FRAG)
            {
                // Allocate from the end of free_block
                allocated_block = reinterpret_cast<heap_rec_t*>(reinterpret_cast<char*>(free_block) + free_block->size - size);
                allocated_block->size = size;
                allocated_block->index = index;
                
                free_block->size -= size + sizeof(heap_rec_t);
                allocated_block->prev = free_block->size;
                
                return allocated_block;
            }
            
            // Too small to split - take all.
            free_block->index = index;
            *ptr = free_block->next;

            return free_block;
        }
    }
    
    return NULL;
}

heap_t::heap_rec_t* heap_t::get_new_block(size_t size, int index)
{
    heap_rec_t* new_free_block;

    if (index != OTHER_INDEX)
        size = all_sizes[index];

    if ((new_free_block = get_new_block_internal(size, index)))
        return new_free_block;

    // coalesce some blocks to free up unfragmented space.
    coalesce();

    if ((new_free_block = get_new_block_internal(size, index)))
        return new_free_block;

    return 0;
}

void *heap_t::allocate(size_t size)
{
    ASSERT(has_lock());
#if HEAP_DEBUG
//    kconsole << "Heap check before allocate(" << size << ")" << endl;
//    check_integrity();
#endif
    int index;
    heap_rec_t* free_block;

    if (size == 0)
        return null_malloc;

    size = BLOCK_ALIGN(size);
    index = find_index(size);
    free_block = blocks[index];
    
    if ((!free_block) || (index == OTHER_INDEX))
    {
        free_block = get_new_block(size, index);
        if (!free_block)
            return NULL;
    }
    else
    {
        blocks[index] = free_block->next;
    }
    
    free_block->heap = this;
    next_block(free_block)->prev = HEAP_MAGIC;

#if HEAP_DEBUG
//    kconsole << "Heap check after allocate(" << size << ")" << endl;
//    check_integrity();
#endif

    kconsole << "heap_t::allocate("<<size<<") returning "<<(free_block+1)<<endl;
    return free_block + 1;
}

void heap_t::free(void *p)
{
    ASSERT(has_lock());
#if HEAP_DEBUG
//    kconsole << "Heap check before free(" << p << ")" << endl;
//    check_integrity();
#endif
    heap_rec_t* to_free;
    heap_rec_t* nextblock;

    // Exit gracefully for null pointers.
    if ((p == NULL) || (p == null_malloc))
    {
        return;
    }
    
    to_free = reinterpret_cast<heap_rec_t*>(p) - 1;
    kconsole << "heap_t::free("<<p<<") freeing "<<to_free<<endl;
    nextblock = next_block(to_free);
    
    to_free->next = blocks[to_free->index];
    blocks[to_free->index] = to_free;

    nextblock->prev = to_free->size;
    
#if HEAP_DEBUG
//    kconsole << "Heap check after free(" << p << ")" << endl;
//    check_integrity();
#endif
}

void* heap_t::realloc(void *ptr, size_t size)
{
    debugger_t::checkpoint("heap_t::realloc");
    UNUSED(size);
    return ptr;
}

void heap_t::expand(size_t new_size)
{
/*
#if HEAP_DEBUG
    check_integrity();
#endif
    // Sanity check.
    ASSERT(new_size > size());

    kconsole << "Heap expanding from " << int(size()) << " to " << int(new_size) << endl;

    // Get the nearest following page boundary.
    new_size = page_align_up<size_t>(new_size);

    // Make sure we are not overreaching ourselves.
    ASSERT(start_address + new_size <= max_address);

    // This should always be on a page boundary.
    uint32_t old_size = size();

    uint32_t i = old_size;
    while(i < new_size)
    {
        nucleus.mem_mgr().page_frame_allocator().alloc_frame(nucleus.mem_mgr().get_kernel_directory()->mapping(start_address+i), is_kernel);
        i += PAGE_SIZE;
    }

    end_address = start_address + new_size;
#if HEAP_DEBUG
    check_integrity();
#endif
*/
}

size_t heap_t::contract(size_t new_size)
{
/*
#if HEAP_DEBUG
    check_integrity();
#endif
    // Sanity check.
    ASSERT(new_size < size());

    // get the nearest following page boundary.
    new_size = page_align_up<size_t>(new_size);

    // Don't contract too far.
    if (new_size < HEAP_MIN_SIZE)
        new_size = HEAP_MIN_SIZE;

    // Not gonna contract anymore anyway
    if (size() == HEAP_MIN_SIZE)
        return size();

    kconsole << "Heap contracting from " << int(size()) << " to " << int(new_size) << endl;

    // Make sure we are not overreaching ourselves.
    ASSERT(new_size > 0);

    uint32_t old_size = size();

    uint32_t i = new_size;
    while(i < old_size)
    {
        nucleus.mem_mgr().page_frame_allocator().free_frame(nucleus.mem_mgr().get_kernel_directory()->mapping(start_address+i));
        i += PAGE_SIZE;
    }

    end_address = start_address + new_size;
#if HEAP_DEBUG
    check_integrity();
#endif
*/
    return new_size;
}

void heap_t::check_integrity()
{
#if HEAP_DEBUG
    // We should, by starting at start_address be able to walk through all blocks/holes and check their magic numbers.
    heap_rec_t* last_header = NULL;
    heap_rec_t* this_header = reinterpret_cast<heap_rec_t*>(start_address);
    heap_rec_t* next_header = next_block(this_header);

    void *end = reinterpret_cast<void*>(end_address);

    if (next_header >= end)
        next_header = NULL;

    kconsole << ">= Heap: starting heap check" << endl;

    while (this_header)
    {
        kconsole << "Heap: checking block " << this_header << endl;
        if (this_header->index > OTHER_INDEX)
        {
            kconsole << LIGHTRED << "Heap integrity check: free list index " << this_header->index << " in block " << this_header << " is invalid." << endl;
            PANIC("Heap corruption!");
        }
        if (start_address + this_header->size >= end_address)
        {
            kconsole << LIGHTRED << "Heap integrity check: block " << this_header << " size " << int(this_header->size) << " is invalid." << endl;
            PANIC("Heap corruption!");
        }

        if (!next_header)
            break;

        if (next_header->prev == HEAP_MAGIC)
        {
            // this_block is allocated.
            kconsole << "Heap: block is ALLOCATED!" << endl
                     << "  prev: " << int(this_header->prev) << endl
                     << "  size: " << int(this_header->size) << endl
                     << "  index: " << this_header->index << endl
                     << "  heap: " << this_header->heap << endl;
        }
        else
        {
            // this block is free and should be in the free list.
            kconsole << "Heap: block is FREE!" << endl
                     << "  prev: " << int(this_header->prev) << endl
                     << "  size: " << int(this_header->size) << endl
                     << "  index: " << this_header->index << endl
                     << "  next: " << this_header->next << endl;
        }
        
        last_header = this_header;
        UNUSED(last_header);
        this_header = next_header;
        next_header = next_block(this_header);
        if (next_header >= end)
            next_header = NULL;
    }
    //TODO: check free-lists
    kconsole << "<= Heap: completed heap check." << endl;
#endif
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
