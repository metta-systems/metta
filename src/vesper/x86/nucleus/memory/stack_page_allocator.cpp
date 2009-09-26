//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "stack_page_allocator.h"
#include "default_console.h"
#include "page_directory.h"
#include "memory.h"
#include "panic.h"

/*
Brendan@osdev:

For pages that are above 0x0000000100000000 you'd need to use 8 bytes (64-bit physical addresses). However,
it's a good idea to use 2 free page stacks (one for pages below 0x0000000100000000 and one for pages
above 0x0000000100000000), because sometimes you need to allocate a page that has a 32-bit physical address
(e.g. for a "page directory pointer table" when you're using PAE, or for some 32-bit PCI devices,
or for some 64-bit PCI devices that are behind a dodgy "32-bit only" PCI to PCI bridge).

However, why stop at just 2 free page stacks? For each free page stack you'll probably need a re-entrancy lock,
and you don't want all the CPUs (potentially) fighting for the same lock at the same time.
If you increase the number of free page stacks for no reason at all, then you'd have more locks and less lock
contention, and because of that you'll get better performance on "many CPU" systems (with almost no extra overhead).

However, if you're going to have lots of free page stacks, why not use them for different things? It's easy to
implement page colouring by having one free page stack per page/cache colour, and that will improve performance
by improving cache efficiency. It's also possible to have a different group of free page stacks for each NUMA
domain (to improve performance by minimizing "distant" RAM accesses), or have different groups of free page stacks
for other purposes.

The previous version of my OS used 1024 free page stacks (but the new version will probably use many more).
At 8 or 16 bytes per free page stack I could probably have millions of them without any problem; but at
4104 bytes per free page stack it starts to get expensive... ;)
*/

stack_page_frame_allocator_t::stack_page_frame_allocator_t()
    : page_frame_allocator_impl_t()
    , lockable_t()
    , next_free_phys(0)
    , total_frames(0)
    , free_frames(0)
    , reserved_frames(0)
{
    kconsole << GREEN << "stacked frame allocator: ctor" << endl;
}

template <typename address_type>
struct area_t
{
    address_type start, end;
    size_t npages(); // type_traits<address_type>::size_type
};

typedef area_t<address_t> alloc_area_t;

void mmap_to_regions(multiboot_t::mmap_t* mmap)
{
#define N_AREAS 32
    alloc_area_t areas[N_AREAS];
    uint32_t n_areas = 1;

    UNUSED(mmap);
    UNUSED(areas);
    UNUSED(n_areas);
    // Start with 1 region covering whole available memory.
    // Use memory map to cut bits off of left or right edge of region
    // or split it up in two.
}

void stack_page_frame_allocator_t::init(multiboot_t::mmap_t* mmap)
{
    kconsole << GREEN << "stacked frame allocator: init " << (address_t)mmap << endl;

    // Go through available physical memory frames, add them to the frame stack.
    ASSERT(mmap);

    // We have created a dent in our memory map, so we need to sort it
    // and build contiguous allocation regions.
    mmap_to_regions(mmap);

    // map different physical pages into single linear address and update their "next_free_phys" pointer.
    multiboot_t::mmap_entry_t* mmi = mmap->first_entry();
    while (mmi)
    {
        address_t start = page_align_up(mmi->address());
        address_t end   = page_align_down(mmi->address() + mmi->size());
        uint32_t n_frames = (end - start) / PAGE_SIZE;

        if (mmi->is_free())
        {
            kconsole << RED << "free frame found" << endl;
            // include pages into free stack
            for (uint32_t n = 0; n < n_frames; n++)
            {
                free_frame(start);
                start += PAGE_SIZE;
            }
        }
        else
        {
            reserved_frames += n_frames;
        }

        total_frames += n_frames;
        mmi = mmap->next_entry(mmi);
    }
    kconsole << "Stack page frame allocator: detected " << (int)total_frames << " frames (" << (int)(total_frames*PAGE_SIZE) << "KB) of physical memory, " << (int)reserved_frames << " frames reserved, " << (int)free_frames << " (" << (int)(free_frames*PAGE_SIZE) << "KB) frames free." << endl;
}

void stack_page_frame_allocator_t::alloc_frame(page_t* p, bool is_kernel, bool is_writeable)
{
    ASSERT(p);
    ASSERT(!p->frame());

    p->set_present(true);
    p->set_writable(is_writeable);
    p->set_user(!is_kernel);
    p->set_frame(alloc_frame());
}

void stack_page_frame_allocator_t::free_frame(page_t* p)
{
    ASSERT(p);
    ASSERT(p->frame());

    free_frame(p->frame());
    p->set_frame(0);
}

address_t stack_page_frame_allocator_t::alloc_frame()
{
    lock();
    address_t next_frame = next_free_phys;
///     mapping_enter(PAGE_MASK, next_frame);
    next_free_phys = *(address_t*)PAGE_MASK;
    free_frames--;
    ASSERT(free_frames <= total_frames);// catch underflow
    unlock();
    return next_frame;
}

// phys_frame becomes the new free stack top.
void stack_page_frame_allocator_t::free_frame(address_t phys_frame)
{
    // map the topmost address space page temporarily to build free stacks
//     mapping_enter(PAGE_MASK, phys_frame);

    lock();
    *(address_t*)PAGE_MASK = next_free_phys; // store phys of previous free stack top
    next_free_phys = phys_frame; // remember phys as current free stack top
    //   unmap_page(PAGE_MASK);
    free_frames++;
    ASSERT(free_frames <= total_frames);// catch overflow
    unlock();
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
