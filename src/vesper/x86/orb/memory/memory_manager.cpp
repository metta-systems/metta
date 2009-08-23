//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "memory_manager.h"
#include "registers.h"
#include "memutils.h"

extern address_t image_end; // defined by linker

/*!
* @internal
* Paging works by splitting the virtual address space into blocks
* called \c pages, which are usually 4KB in size. Pages can then
* be mapped on to \c frames - equally sized blocks of physical memory.
*/

memory_manager_t& memory_manager_t::self()
{
    static memory_manager_t manager;
    return manager;
}

memory_manager_t::memory_manager_t()
{
    placement_address = (address_t)&image_end;
    heap_initialised = false;
    current_directory = kernel_directory = NULL;
}

void memory_manager_t::init(address_t mem_end, multiboot_t::header_t::memmap_t* mmap)
{
    frame_allocator.init(mem_end, mmap);

    // Make a page directory.
    kernel_directory = new(true/*page align*/) page_directory();
    current_directory = kernel_directory;

    // Map kernel code

    // Map some pages in the kernel heap area.
    // Here we call getPage but not alloc_frame. This causes PageTables
    // to be created where nessecary. We can't allocate frames yet because
    // they need to be identity mapped first below.
    for (uint32_t i = HEAP_START; i < HEAP_END; i += PAGE_SIZE)
    {
        kernel_directory->get_page(i, /*make:*/true);
    }

    // Identity map from KERNEL_START to placementAddress.
    // reserve 16 pages above placement_address so that we can use placement alloc at start.
    uint32_t i = 0;
    while (i < placement_address + 16*PAGE_SIZE)
    {
        // Kernel code is readable but not writable from userspace.
        alloc_frame(kernel_directory->get_page(i, true) , /*kernel:*/false, /*writable:*/false);
        i += PAGE_SIZE;
    }

    // Now allocate those pages we mapped earlier.
    for (i = K_HEAP_START; i < K_HEAP_START + K_HEAP_INITIAL_SIZE; i += PAGE_SIZE)
    {
        // Kernel heap is readable but not writable from userspace.
        alloc_frame(kernel_directory->get_page(i, true), false, false);
    }

//FIXME paging is already enabled by kickstart
//     ia32_mmu_t::set_active_pagetable((address_t)kernel_directory->get_physical());
//     ia32_mmu_t::enable_paged_mode();

    // Initialise the heaps.
    heap.init(K_HEAP_START, K_HEAP_START + K_HEAP_INITIAL_SIZE, K_HEAP_END & PAGE_MASK /* see memory map */, true);

    heap_initialised = true;
}

void* memory_manager_t::allocate(uint32_t size, bool page_align, address_t* physical_address)
{
    ASSERT(heap_initialised);
    heap.lock();
    void* addr = heap.allocate(size, page_align);
    heap.unlock();
    if (physical_address)
    {
        page_t* pg = kernel_directory->get_page((address_t)addr, false);
        *physical_address = pg->frame() + (address_t)addr % PAGE_SIZE;
    }
    return addr;
}

void* memory_manager_t::reallocate(void* ptr, size_t size)
{
    ASSERT(heap_initialised);
    heap.lock();
    void* p = heap.realloc(ptr, size);
    heap.unlock();
    return p;
}

void memory_manager_t::free(void* p)
{
    ASSERT(heap_initialised);
    heap.lock();
    heap.free(p);
    heap.unlock();
}

void memory_manager_t::align_placement_address()
{
    if (placement_address % PAGE_SIZE) // if it needs aligning at all!
    {
        placement_address += PAGE_SIZE - placement_address % PAGE_SIZE;
    }
}

void memory_manager_t::allocate_range(UNUSED_ARG address_t start_address, UNUSED_ARG address_t size)
{
}

uint32_t memory_manager_t::get_heap_size()
{
    return heap.size();
}

void memory_manager_t::check_integrity()
{
    // TODO: heap should be locked
    if(heap_initialised)
    {
        heap.check_integrity();
    }
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
