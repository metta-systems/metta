//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2010, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "x86_frame_allocator.h"
#include "config.h"
#include "mmu.h"
#include "ia32.h"
#include "cpu.h"
#include "page_directory.h"
#include "memutils.h" //runtime/
#include "default_console.h"
#include "linksyms.h"
#include "stl/algorithm"

#define TEMP_MAPPING (VIRTUAL_PAGE_TABLES - PAGE_SIZE)
extern "C" address_t _kickstart_begin;

x86_frame_allocator_t x86_frame_allocator_t::allocator_instance;
physical_address_t x86_frame_allocator_t::allocation_address = 0;

frame_allocator_t& frame_allocator_t::instance()
{
    return x86_frame_allocator_t::instance();
}

x86_frame_allocator_t& x86_frame_allocator_t::instance()
{
    return allocator_instance;
}

x86_frame_allocator_t::x86_frame_allocator_t()
    : lockable_t()
    , stack_initialised(false)
    , reserved_area_start(LINKSYM(_kickstart_begin))
{
#if MEMORY_DEBUG
    kconsole << GREEN << "x86_frame_allocator: ctor" << endl;
#endif
}

frame_allocator_t::memory_range_t x86_frame_allocator_t::reserved_range()
{
    return frame_allocator_t::memory_range_t(reinterpret_cast<void*>(reserved_area_start), 0, allocation_address - reserved_area_start, "reserved during boot");
}

/* Build memory-ranges and page stacks before paging is enabled, to avoid mapping frames. */
void x86_frame_allocator_t::initialise_before_paging(multiboot_t::mmap_t* mmap, memory_range_t reserved_boot_range)
{
    range_list_t<physical_address_t> free_ranges, reserved_ranges;

    ASSERT(mmap);
    multiboot_t::mmap_entry_t* mmi = mmap->first_entry();
    while (mmi)
    {
        if (mmi->is_free())
            free_ranges.free(mmi->address(), mmi->size());
        else
            reserved_ranges.free(mmi->address(), mmi->size());
        mmi = mmap->next_entry(mmi);
    }

    // Preserve the currently executing kickstart code in the memory allocator init.
    // We will give up these frames later.
    free_ranges.allocate(reinterpret_cast<address_t>(reserved_boot_range.virtual_address), reserved_boot_range.size);

#if MEMORY_DEBUG
    kconsole << GREEN << "x86_frame_allocator: init " << (address_t)mmap << " reserved from " << reserved_boot_range.virtual_address << " for " << reserved_boot_range.size << " bytes" << endl;
#endif

    foreach(auto range, free_ranges)
    {
        address_t start  = page_align_up(range->start);
        address_t finish = page_align_down(start + range->length);
        size_t n_frames = (finish - start) / PAGE_SIZE;
        page_stack_t& stk = stack(start, finish - start);
        stk.total_frames += n_frames;

        kconsole << "mmap entry: start " << start << ", end " << finish << ", size " << range->length << " (" << n_frames << " frames)" << endl;

        // include pages into free stack
        for (size_t n = 0; n < n_frames; n++)
        {
            *reinterpret_cast<address_t*>(start) = stk.next_free_phys; // store phys of previous free stack top
            stk.next_free_phys = start; // remember phys as current free stack top
            stk.free_frames++;
//             kconsole << "adding " << start << " to stack" << endl;
            start += PAGE_SIZE;
        }
    }

    foreach(auto range, reserved_ranges)
    {
        address_t start  = page_align_up(range->start);
        address_t finish = page_align_down(start + range->length);
        size_t n_frames = (finish - start) / PAGE_SIZE;
        page_stack_t& stk = stack(start, finish - start);
        stk.total_frames += n_frames;
        stk.reserved_frames += n_frames;
    }

    kconsole << "x86_frame_allocator_t:" << endl
             << " below 1Mb: ";
    {
    page_stack_t& stk = stack(0, 1);
    kconsole << "total " << (int)stk.total_frames << " frames (" << (int)(stk.total_frames*PAGE_SIZE/1024) << "KB), " << (int)stk.reserved_frames << " frames (" << (int)(stk.reserved_frames*PAGE_SIZE/1024) << "KB) reserved, " << (int)stk.free_frames << " frames (" << (int)(stk.free_frames*PAGE_SIZE/1024) << "KB) free." << endl
             << " below 16Mb: ";
    }
    {
        page_stack_t& stk = stack(1*MB, 1);
    kconsole << "total " << (int)stk.total_frames << " frames (" << (int)(stk.total_frames*PAGE_SIZE/1024) << "KB), " << (int)stk.reserved_frames << " frames (" << (int)(stk.reserved_frames*PAGE_SIZE/1024) << "KB) reserved, " << (int)stk.free_frames << " frames (" << (int)(stk.free_frames*PAGE_SIZE/1024) << "KB) free." << endl
             << " above 16Mb: ";
    }
    {
        page_stack_t& stk = stack(16*MB, 1);
    kconsole << "total " << (int)stk.total_frames << " frames (" << (int)(stk.total_frames*PAGE_SIZE/1024) << "KB), " << (int)stk.reserved_frames << " frames (" << (int)(stk.reserved_frames*PAGE_SIZE/1024) << "KB) reserved, " << (int)stk.free_frames << " frames (" << (int)(stk.free_frames*PAGE_SIZE/1024) << "KB) free." << endl;
    }

    stack_initialised = true;
}

void x86_frame_allocator_t::initialisation_complete()
{
}

x86_frame_allocator_t::page_stack_t& x86_frame_allocator_t::stack(physical_address_t address, size_t size)
{
    if ((address < 1*MB) && (address+size <= 1*MB))
        return page_stacks[below_1mb];
    else if ((address < 16*MB) && (address+size <= 16*MB))
        return page_stacks[below_16mb];
    else if (address >= 16*MB)
        return page_stacks[above_16mb];
    else
        PANIC("Range not entirely within 1Mb or 16Mb range.");
}

x86_frame_allocator_t::page_stack_t& x86_frame_allocator_t::first_usable_stack()
{
    page_stack_t& stk = stack(16*MB, 1);
    if (stk.free_frames == 0)
    {
        stk = stack(1*MB, 1);
        if (stk.free_frames == 0)
        {
            stk = stack(0, 1);
            if (stk.free_frames == 0)
                PANIC("Out of memory!"); // TODO: swap here
        }
    }
    return stk;
}

physical_address_t x86_frame_allocator_t::allocate_frame()
{
    if (!stack_initialised)
    {
        allocation_address = page_align_up(allocation_address);
        physical_address_t tmp = allocation_address;
        allocation_address += PAGE_SIZE;
//         stack(tmp, PAGE_SIZE).ranges.allocate(tmp, PAGE_SIZE);
        return tmp;
    }

    lockable_scope_lock_t guard(*this);

    address_t next_frame;

    // Allocate frame is not constrained, so pick first appropriate stack starting from higher memory regions.
    page_stack_t& stk = first_usable_stack();

    if (unlikely(!ia32_mmu_t::paged_mode_enabled()))
    {
        next_frame = stk.next_free_phys;
        stk.next_free_phys = *reinterpret_cast<address_t*>(next_frame);
        // clear frame before use
        memutils::fill_memory(reinterpret_cast<void*>(next_frame), 0, page_size());
    }
    else
    {
        protection_domain_t& domain = x86_cpu_t::current_cpu().current_protection_domain();
//         ASSERT(domain == protection_domain_t::privileged());

        // TODO: update ramtab with owner information here
        // ramtab[entry].owner = domain;
        next_frame = stk.next_free_phys;
        domain.map(next_frame, TEMP_MAPPING, page_t::kernel_mode | page_t::writable);

        stk.next_free_phys = *reinterpret_cast<address_t*>(TEMP_MAPPING);
        // wipe it clean
        memutils::fill_memory(reinterpret_cast<void*>(TEMP_MAPPING), 0, PAGE_SIZE);

        domain.unmap(TEMP_MAPPING);
    }

    stk.ranges.allocate(next_frame, PAGE_SIZE);

    stk.free_frames--;
    ASSERT(stk.free_frames <= stk.total_frames);// catch underflow

    return next_frame;
}

/* need not be concerned with removing possibly existing mappings to this physical, handled by higher level? */
void x86_frame_allocator_t::free_frame(physical_address_t frame)
{
    lockable_scope_lock_t guard(*this);

    page_stack_t& stk = first_usable_stack();

    if (unlikely(!ia32_mmu_t::paged_mode_enabled()))
    {
        *reinterpret_cast<address_t*>(frame) = stk.next_free_phys;
        stk.next_free_phys = frame; // remember phys as current free stack top
    }
    else
    {
        protection_domain_t& domain = x86_cpu_t::current_cpu().current_protection_domain();
//         ASSERT(domain == protection_domain_t::privileged());

        domain.map(frame, TEMP_MAPPING, page_t::kernel_mode | page_t::writable);

        *reinterpret_cast<address_t*>(TEMP_MAPPING) = stk.next_free_phys; // store phys of previous free stack top
        stk.next_free_phys = frame; // remember phys as current free stack top

        domain.unmap(TEMP_MAPPING);
    }

    stk.ranges.free(frame, PAGE_SIZE);

    stk.free_frames++;
    ASSERT(stk.free_frames <= stk.total_frames);// catch overflow
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

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
