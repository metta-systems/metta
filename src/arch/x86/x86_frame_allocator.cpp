//
// Copyright 2007 - 2010, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "config.h"
#include "memory.h" //common/
#include "memutils.h" //runtime/
#include "x86_frame_allocator.h"
#include "default_console.h"

#define TEMP_MAPPING (RPAGETAB_VBASE - PAGE_SIZE)

x86_frame_allocator_t x86_frame_allocator_t::instance_;

x86_frame_allocator_t::x86_frame_allocator_t()
    : lockable_t()
    , next_free_phys(0)
    , total_frames(0)
    , free_frames(0)
    , reserved_frames(0)
{
#if MEMORY_DEBUG
    kconsole << GREEN << "x86_frame_allocator: ctor" << endl;
#endif
}

/* Build memory-ranges and page stacks before paging is enabled, to avoid mapping frames. */
void x86_frame_allocator_t::initialise_before_paging(multiboot_t::mmap_t* mmap, memory_range_t reserved_boot_range)
{
    range_list_t<address_t> free_ranges;

    ASSERT(mmap);
    multiboot_t::mmap_entry_t* mmi = mmap->first_entry();
    while (mmi)
    {
        if (mmi->is_free())
            free_ranges.free(mmi->size(), mmi->address());
        mmi = mmap->next_entry(mmi);
    }

    // Preserve the currently executing kickstart code in the memory allocator init.
    // We will give up these frames later.
    free_ranges.allocate(reserved_boot_range.size, reinterpret_cast<address_t>(reserved_boot_range.virtual_address));

#if MEMORY_DEBUG
    kconsole << GREEN << "x86_frame_allocator: init " << (address_t)mmap << " reserved from " << reserved_boot_range.virtual_address << " for " << reserved_boot_range.size << " bytes" << endl;
#endif

    next_free_phys = 0;
    auto cur(free_ranges.begin());
    auto end(free_ranges.end());
    for (; cur != end; ++cur)
    {
        address_t start = page_align_up((*cur)->start);
        address_t end   = page_align_down(start + (*cur)->length);
        size_t n_frames = (end - start) / PAGE_SIZE;
        total_frames += n_frames;

        kconsole << "mmap entry: start " << start << ", end " << end << ", size " << (*cur)->length << endl;

        // include pages into free stack
        for (size_t n = 0; n < n_frames; n++)
        {
            *reinterpret_cast<address_t*>(start) = next_free_phys; // store phys of previous free stack top
            next_free_phys = start; // remember phys as current free stack top
            free_frames++;
            kconsole << "adding " << start << " to stack" << endl;
            start += PAGE_SIZE;
        }
    }

    kconsole << "x86_frame_allocator_t: detected " << (int)total_frames << " frames (" << (int)(total_frames*PAGE_SIZE/1024) << "KB) of physical memory, " << (int)reserved_frames << " frames (" << (int)(reserved_frames*PAGE_SIZE/1024) << "KB) reserved, " << (int)free_frames << " frames (" << (int)(free_frames*PAGE_SIZE/1024) << "KB) free." << endl;
}

void x86_frame_allocator_t::initialisation_complete()
{
}

physical_address_t x86_frame_allocator_t::allocate_frame()
{
    lock();
    address_t next_frame = next_free_phys;
//     protection_domain_t* domain = processor_t::current_cpu().current_protection_domain();
//     ASSERT(domain == protection_domain_t::privileged());
//     domain->map(next_frame, TEMP_MAPPING, frame_t::writable|frame_t::kernel);

    next_free_phys = *reinterpret_cast<address_t*>(TEMP_MAPPING);
    free_frames--;
    ASSERT(free_frames <= total_frames);// catch underflow

    // wipe it clean
    memutils::fill_memory(reinterpret_cast<void*>(TEMP_MAPPING), 0, PAGE_SIZE);

    unlock();

//     domain->unmap(TEMP_MAPPING);
    return next_frame;
}

/* need not be concerned with removing possibly existing mappings to this physical, handled by higher level? */
void x86_frame_allocator_t::free_frame(physical_address_t frame)
{
    lock();
//     protection_domain_t* domain = processor_t::current_cpu().current_protection_domain();
//     ASSERT(domain == protection_domain_t::privileged());
//     domain->map(frame, TEMP_MAPPING, frame_t::writable|frame_t::kernel);

    *reinterpret_cast<address_t*>(TEMP_MAPPING) = next_free_phys; // store phys of previous free stack top
    next_free_phys = frame; // remember phys as current free stack top
    free_frames++;
    ASSERT(free_frames <= total_frames);// catch overflow

//     domain->unmap(TEMP_MAPPING);
    unlock();
}

bool x86_frame_allocator_t::allocate_range(memory_range_t& /*range*/,
                                size_t /*num_frames*/,
                                flags_t /*page_constraints*/,
                                flags_t /*flags*/,
                                physical_address_t /*start*/)
{
    return false;
}

bool x86_frame_allocator_t::free_range(memory_range_t& /*range*/)
{
    return false;
}

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
