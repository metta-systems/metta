//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "memory_manager.h"
#include "registers.h"
#include "memutils.h"
#include "stack_page_allocator.h"
#include "page_directory.h"
#include "new.h"

extern address_t image_end; // defined by linker

/*!
* @internal
* Paging works by splitting the virtual address space into blocks
* called @a pages, which are usually 4KB in size. Pages can then
* be mapped onto @a frames - equally sized blocks of physical memory.
*/

static stack_page_frame_allocator_t stack_allocator;

memory_manager_t::memory_manager_t()
    : frame_allocator(&stack_allocator)
{
    kconsole << GREEN << "memory_manager: ctor" << endl;
    placement_address = (address_t)&image_end; // NOT PHYSICAL HERE, FIX IT by using frame_allocator
    // try to not use placement_address at all, init heap early.

    heap_initialized = false;
    current_directory = kernel_directory = NULL;
}

void memory_manager_t::init(multiboot_t::mmap_t* mmap, kickstart_n::memory_allocator_t* mmgr)
{
    kconsole << GREEN << "memory_manager: init " << (address_t)mmgr << endl;
    frame_allocator.init(mmap, mmgr);
    // now we can allocate frames of physical memory

    // Make a page directory.
    kernel_directory = new(true) page_directory_t();
    current_directory = kernel_directory;

    // Map kernel code (mapped by elf loader already).

    // Map some pages in the kernel heap area.
    // Here we call get_page but not alloc_frame. This causes pagetables
    // to be created where nessecary. We can't allocate frames yet because
    // they need to be identity mapped first below.
    for (uint32_t i = K_HEAP_START; i < K_HEAP_END; i += PAGE_SIZE)
    {
        kernel_directory->get_page(i, /*make:*/true);
    }

    // Identity map from KERNEL_START to placementAddress.
    // reserve 16 pages above placement_address so that we can use placement alloc at start.
//     uint32_t i = 0;
//     while (i < placement_address + 16*PAGE_SIZE)
//     {
        // TODO Kernel code is readable but not writable from userspace.
//         frame_allocator.alloc_frame(kernel_directory->get_page(i, true) , /*kernel:*/false, /*writable:*/false);
//         i += PAGE_SIZE;
//     }

    // Now allocate those pages we mapped earlier.
    for (uint32_t i = K_HEAP_START; i < K_HEAP_START + K_HEAP_INITIAL_SIZE; i += PAGE_SIZE)
    {
        // Kernel heap is readable but not writable from userspace.
        frame_allocator.alloc_frame(kernel_directory->get_page(i, true), false, false);
    }

//FIXME paging is already enabled by kickstart
//     ia32_mmu_t::set_active_pagetable((address_t)kernel_directory->get_physical());
//     ia32_mmu_t::enable_paged_mode();

    // Initialise the heaps.
    heap.init(K_HEAP_START, K_HEAP_START + K_HEAP_INITIAL_SIZE, K_HEAP_END & PAGE_MASK, true);

    heap_initialized = true;
}

void* memory_manager_t::allocate(uint32_t size, bool page_align, address_t* physical_address)
{
    ASSERT(heap_initialized);
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
    ASSERT(heap_initialized);
    heap.lock();
    void* p = heap.realloc(ptr, size);
    heap.unlock();
    return p;
}

void memory_manager_t::free(void* p)
{
    ASSERT(heap_initialized);
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
    if(heap_initialized)
    {
        heap.lock();
        heap.check_integrity();
        heap.unlock();
    }
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
