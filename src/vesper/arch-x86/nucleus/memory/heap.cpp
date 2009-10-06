//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "heap.h"
#include "memory.h"
#include "memory_manager.h"
#include "page_directory.h"
#include "nucleus.h"
#include "linksyms.h"
#include "config.h" // for HEAP_DEBUG

using nucleus_n::nucleus;

/*!
* @class heap_t
* XXX be careful not to use memory-allocating kconsole calls inside heap_t
* as heap methods run with heap locked and asking to malloc from inside heap_t
* will deadlock.
*/

#define HEAP_MAGIC        0xbeefbead
#define HEAP_MIN_SIZE     0x70000

void heap_t::init(address_t start, address_t end, address_t max, bool supervisor)
{
    start_address = start;
    end_address   = end;
    max_address   = max;
    is_kernel     = supervisor;

    kconsole.print("Initializing heap_t (%08x..%08x, kernel: %d).\n", start, end, is_kernel);

    ASSERT(is_page_aligned<address_t>(start_address));
    ASSERT(is_page_aligned<address_t>(end_address));

    // Initialise the index.
    index = (ordered_array_t<header_t, HEAP_INDEX_SIZE>*)start_address;

    // Shift the start address to resemble where we can start putting data.
    start_address += sizeof(*index);

    // make sure startAddress is page-aligned.
    start_address = page_align_up<address_t>(start_address);

    // Start by having just one big hole.
    header_t* hole_header = (header_t*)start_address;
    hole_header->size = end_address - start_address;
    hole_header->magic = HEAP_MAGIC;
    hole_header->is_hole = true;
    footer_t* hole_footer = (footer_t*)(start_address + hole_header->size - sizeof(footer_t));
    hole_footer->header = hole_header;
    hole_footer->magic = HEAP_MAGIC;

    index->insert(hole_header);
}

// heap_t::~heap_t()
// {
    //TODO: check that everything was deallocated prior to destroying the heap_t
    //Print leak summary.
// }

// Find the smallest hole that will fit.
int32_t heap_t::find_smallest_hole(size_t size, bool page_align)
{
    uint32_t iterator = 0;
    while (iterator < index->count())
    {
        // if the user has requested the memory be page-aligned
        if (page_align)
        {
            // page-align the starting point of this header_t.
            address_t location = index->lookup<address_t>(iterator);
            int32_t offset = 0;
            if ((location + sizeof(header_t)) % PAGE_SIZE)
            {
                offset = PAGE_SIZE - (location + sizeof(header_t)) % PAGE_SIZE;
            }

            int32_t hole_size = index->lookup(iterator)->size;
            hole_size -= offset;

            // can we fit now?
            if (hole_size >= (int32_t)size)
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

void *heap_t::allocate(size_t size, bool page_align)
{
    ASSERT(has_lock());
#ifdef HEAP_DEBUG
    check_integrity();
#endif

    // Take into account the header_t/footer_t.
    size_t new_size = size + sizeof(header_t) + sizeof(footer_t);

    int32_t iterator = find_smallest_hole(new_size, page_align);

    if (iterator == -1) // If we didn't find a suitable hole
    {
        size_t old_length = end_address - start_address;
        address_t old_end_address = end_address;

        // Allocate some more space.
        expand(old_length + new_size);

        size_t new_length = end_address - start_address;

        // Find the endmost header_t. (Not endmost in size, endmost in location)
        uint32_t iterator2 = 0;
        int32_t idx = -1;
        address_t value = 0x0;

        while (iterator2 < index->count())
        {
            if (index->lookup<address_t>(iterator2) > value)
            {
                value = index->lookup<address_t>(iterator2);
                idx = iterator2;
            }
            iterator2++;
        }

        // If we didnt find ANY headers, we need to add one.
        if (idx == -1)
        {
            header_t *new_header = (header_t *)old_end_address;
            new_header->magic = HEAP_MAGIC;
            new_header->size = new_length - old_length;
            new_header->is_hole = true;
            footer_t *new_footer = (footer_t *)((address_t)new_header + new_header->size - sizeof(footer_t));
            new_footer->magic = HEAP_MAGIC;
            new_footer->header = new_header;
            // Put this new header_t in the index
            index->insert(new_header);
        }
        else
        {
            // The last header_t is the one whose size needs adjusting.
            header_t *last_header = index->lookup(idx);
            last_header->size += new_length - old_length;
            // Rewrite it's footer.
            footer_t* last_footer = (footer_t *)((address_t)last_header + last_header->size - sizeof(footer_t));
            last_footer->magic = HEAP_MAGIC;
            last_footer->header = last_header;
            // And here the sorting order in ordered_array is screwed now. Need to take out and reinsert this item.
            index->remove(idx);
            index->insert(last_header);
        }
#ifdef HEAP_DEBUG
        check_integrity();
#endif
        // TODO: optimize tail-recursion
        return allocate(size, page_align);
    }

    if (index->lookup(iterator)->magic != HEAP_MAGIC)
    {
        kconsole.print("block: %08x\n", index->lookup(iterator));
    }
    ASSERT(index->lookup(iterator)->magic == HEAP_MAGIC);

    address_t orig_hole_pos  = index->lookup<address_t>(iterator);
    size_t orig_hole_size = index->lookup(iterator)->size;

    // If the original hole size minus the requested hole size is less than
    // the space required to make a new hole (sizeof(header_t)+sizeof(footer_t)),
    // then just use the origHoleSize.
    if (orig_hole_size - new_size < sizeof(header_t)+sizeof(footer_t))
    {
        size += orig_hole_size - new_size;
        new_size = orig_hole_size;
    }

    // If we need to page-align the data, do it now and make a new hole.
    if (page_align && (orig_hole_pos % PAGE_SIZE))
    {
        address_t new_location = orig_hole_pos + PAGE_SIZE - orig_hole_pos % PAGE_SIZE - sizeof(header_t);
        header_t *hole_header  = (header_t *)orig_hole_pos;
        hole_header->size    = PAGE_SIZE - orig_hole_pos % PAGE_SIZE - sizeof(header_t);
        hole_header->magic   = HEAP_MAGIC;
        hole_header->is_hole = true;
        footer_t *hole_footer  = (footer_t *)((address_t)new_location - sizeof(footer_t));
        hole_footer->magic   = HEAP_MAGIC;
        hole_footer->header  = hole_header;
        orig_hole_pos        = new_location;
        orig_hole_size       = orig_hole_size - hole_header->size;
    }
    else
    {
        // Delete the hole.
        index->remove(iterator);
    }

    // Overwrite the original header.
    header_t *block_header  = (header_t *)orig_hole_pos;
    block_header->magic   = HEAP_MAGIC;
    block_header->is_hole = false;
    block_header->size    = new_size;

    // And the footer...
    footer_t *block_footer  = (footer_t *)(orig_hole_pos + sizeof(header_t) + size);
    block_footer->magic   = HEAP_MAGIC;
    block_footer->header  = block_header;

    // If the new hole wouldn't have size zero...
    if (orig_hole_size - new_size > 0)
    {
        // Write it.
        header_t *hole_header  = (header_t *)(orig_hole_pos + sizeof(header_t) + size + sizeof(footer_t));
        hole_header->magic   = HEAP_MAGIC;
        hole_header->is_hole = true;
        hole_header->size    = orig_hole_size - new_size;

        footer_t *hole_footer = (footer_t *)((address_t)hole_header + orig_hole_size - new_size - sizeof(footer_t));
        // Check we didn't go outside allowed heap_t area.
        if ((address_t)hole_footer < LINKSYM(K_HEAP_START) || (address_t)hole_footer > LINKSYM(K_HEAP_END))
        {
            kconsole.set_color(LIGHTRED);
            kconsole.print("Footer %p outside bounds!\n orig_hole_size: %d\n new_size: %d\n header_t: %p\n", hole_footer, orig_hole_size, new_size, hole_header);
        }
        if ((address_t)hole_footer < end_address)
        {
            hole_footer->magic  = HEAP_MAGIC;
            hole_footer->header = hole_header;
        }

        // Put the new hole in the index.
        index->insert(hole_header);
    }

#ifdef HEAP_DEBUG
    check_integrity();
#endif

    return (void *)((address_t)block_header + sizeof(header_t));
}

void heap_t::free(void *p)
{
    ASSERT(has_lock());
#ifdef HEAP_DEBUG
    check_integrity();
#endif

    // Exit gracefully for null pointers.
    if (!p)
    {
        return;
    }

    // Get the header_t and footer_t associated with this pointer.
    header_t *block_header = (header_t *)((address_t)p - sizeof(header_t));
    footer_t *block_footer = (footer_t *)((address_t)block_header + block_header->size - sizeof(footer_t));

    // Consistency check...
    if (block_header->magic != HEAP_MAGIC)
    {
        kconsole.set_color(LIGHTRED);
        kconsole.print("Heap header_t %p invalid, magic %08x, size: %d\n", block_header, block_header->magic, block_header->size);
    }
    ASSERT(block_header->magic == HEAP_MAGIC);
    ASSERT(block_footer->magic == HEAP_MAGIC);
    ASSERT(block_footer->header == block_header);
    ASSERT(!block_header->is_hole);

    // Make us a hole.
    block_header->is_hole = true;

    // Do we want to add the header_t into the index?
    bool do_add = true;

    // TODO: Factor out unify left/right for use in realloc()

    // Unify left
    // if the thing immediately to the left of us is a hole footer_t...
    footer_t *test_footer = (footer_t *)((address_t)block_header - sizeof(footer_t));
    if (test_footer->magic == HEAP_MAGIC && test_footer->header->is_hole)
    {
        // cache our current size.
        size_t cached_size = block_header->size;
        // rewrite our header_t with the new one
        block_header = test_footer->header;
        // rewrite our footer_t to point to the new header_t.
        block_footer->header = block_header;
        // change the size.
        block_header->size += cached_size;
        // Since this header_t is already in the index, we don't want to add it again.
        do_add = false; // FIXME: sorting by size will get skewed? (also, postpone reinserting modified header_t until unify right is finished)
    }

    // Unify right
    // if the the thing immediately to the right of us is a hole header_t...
    header_t *test_header = (header_t *)((address_t)block_footer + sizeof(footer_t));
    if (test_header->magic == HEAP_MAGIC && test_header->is_hole)
    {
        // increase our size.
        block_header->size += test_header->size;

        // rewrite its footer_t to point to our header_t.
        test_footer = (footer_t *)((address_t)test_header + test_header->size - sizeof(footer_t));
        ASSERT(test_footer->magic == HEAP_MAGIC); // there should be a footer_t down there
        ASSERT(test_footer->header == test_header);
        test_footer->header = block_header;

        // It's now OUR footer_t! muahahaha....
        block_footer = test_footer;

        // find and remove this header_t from the index.
        uint32_t iterator = 0;
        // TODO: build index on header_t address?
        while ((iterator < index->count()) && (index->lookup(iterator) != test_header))
        {
            iterator ++;
        }

        // Make sure we actually found the item.
        ASSERT(iterator < index->count());

        // Remove it.
        index->remove(iterator);
    }

    // If the footer_t location is the end address, we can contract.
    if ((address_t)block_footer + sizeof(footer_t) == end_address)
    {
        uint32_t old_length = size();
        uint32_t new_length = contract((address_t)block_header - start_address);

        // Check how big we will be after resizing
        if (block_header->size - (old_length - new_length) > 0)
        {
            // we still exist, resize us.
            block_header->size -= old_length - new_length;
            block_footer = (footer_t *)((address_t)block_header + block_header->size - sizeof(footer_t));
            block_footer->magic = HEAP_MAGIC;
            block_footer->header = block_header;
        }
        else
        {
            // We no longer exist :(. Remove us from the index.
            uint32_t iterator = 0;
            while ((iterator < index->count()) && (index->lookup(iterator) != block_header))
            {
                iterator ++;
            }

            // If we didnt find ourselves, we have nothing to remove.
            if (iterator < index->count())
            {
                index->remove(iterator);
            }

            // ...and nothing to add.
            do_add = false;
        }
    }

    // Add us to the index
    if (do_add)
    {
        index->insert(block_header);
    }

#ifdef HEAP_DEBUG
    check_integrity();
#endif
}

void* heap_t::realloc(void *ptr, size_t size)
{
    kconsole.checkpoint("heap_t::realloc");
    (void)size;
    return ptr;
}

void heap_t::expand(size_t new_size)
{
#ifdef HEAP_DEBUG
    check_integrity();
#endif
    // Sanity check.
    ASSERT(new_size > size());

    kconsole.print("Heap expanding from %d to %d\n", size(), new_size);

    // Get the nearest following page boundary.
    new_size = page_align_up<size_t>(new_size);

    // Make sure we are not overreaching ourselves.
    ASSERT(start_address + new_size <= max_address);

    // This should always be on a page boundary.
    uint32_t old_size = size();

    uint32_t i = old_size;
    while(i < new_size)
    {
        nucleus.mem_mgr().page_frame_allocator().alloc_frame(nucleus.mem_mgr().get_kernel_directory()->get_page(start_address+i), is_kernel);
        i += PAGE_SIZE;
    }

    end_address = start_address + new_size;
#ifdef HEAP_DEBUG
    check_integrity();
#endif
}

size_t heap_t::contract(size_t new_size)
{
#ifdef HEAP_DEBUG
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

    kconsole << "Heap contracting from " << size() << " to " << new_size << endl;

    // Make sure we are not overreaching ourselves.
    ASSERT(new_size > 0);

    uint32_t old_size = size();

    uint32_t i = new_size;
    while(i < old_size)
    {
        nucleus.mem_mgr().page_frame_allocator().free_frame(nucleus.mem_mgr().get_kernel_directory()->get_page(start_address+i));
        i += PAGE_SIZE;
    }

    end_address = start_address + new_size;
#ifdef HEAP_DEBUG
    check_integrity();
#endif
    return new_size;
}

void heap_t::check_integrity()
{
#ifdef HEAP_DEBUG
    // We should, by starting at start_address, be able to walk through all blocks/
    // holes and check their magic numbers.
    header_t *last_header = NULL;
    header_t *this_header = (header_t*)start_address;
    header_t *next_header = (header_t*)((address_t)this_header + this_header->size);
    if ((address_t)next_header >= end_address)
        next_header = NULL;

    while (this_header)
    {
        // header_t overwritten.
        if (this_header->magic != HEAP_MAGIC)
        {
            kconsole.set_color(LIGHTRED);
            kconsole.print(
            "\nPrevious block:\n  Address: %p\n  Size: %d\n  Hole: %d\n"
            "This block:\n  Address: %p\n  Size: %d\n  Hole: %d\n",
            last_header, last_header->size, last_header->is_hole,
            this_header, this_header->size, this_header->is_hole);
            PANIC("Heap header_t overwritten!");
        }

        if (!next_header)
            break;

        footer_t *this_footer = (footer_t*)((address_t)next_header - sizeof(footer_t));
        // footer_t overwritten.
        if (this_footer->magic != HEAP_MAGIC)
        {
            kconsole.set_color(LIGHTRED);
            kconsole.print(
            "\nPrevious block:\n  Address: %p\n  Size: %d\n  Hole: %d\n"
            "This block:\n  Address: %p\n  Size: %d\n  Hole: %d\n",
            "Next block:\n  Address: %p\n  Size: %d\n  Hole: %d\n",
            last_header, last_header->size, last_header->is_hole,
            this_header, this_header->size, this_header->is_hole,
            next_header, next_header->size, next_header->is_hole);
            PANIC("Heap footer_t overwritten!");
        }

        last_header = this_header;
        this_header = next_header;
        next_header = (header_t*)((address_t)this_header + this_header->size);
        if ((uint32_t)next_header >= end_address)
            next_header = NULL;
    }
#endif
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
