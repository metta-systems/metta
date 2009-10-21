//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "types.h"
#include "heap.h"
#include "memory.h"
#include "multiboot.h"
#include "page_directory.h"
#include "nucleus_frame_allocator.h"

#define K_HEAP_INITIAL_SIZE     0x100000
#define STACK_INITIAL_SIZE      0x10000

#define STACK_ADDRESS(x)  ((address_t)x <= K_STACK_START && (address_t)x > K_STACK_END)
#define K_HEAP_ADDRESS(x) ((address_t)x >= K_HEAP_START && (address_t)x <= K_HEAP_END)

// Last 4 megs of address space are for recursive page directory
#define RPAGETAB_VBASE 0xffc00000 // page tables addressable from this base
#define RPAGEDIR_VBASE 0xfffff000 // page directory addressable from this base

class page_t;
class page_directory_t;

/*!
* Kernel memory manager controls physical and virtual memory allocation.
*/
class memory_manager_t
{
public:
    /*!
    * Default constructor, does nothing.
    */
    memory_manager_t();

    /*!
    * Normal constructor.
    * Initialises frame allocator, then maps some pages for the heap.
    */
    void init(multiboot_t::mmap_t* mmap);

    /*!
    * Allocate @c size bytes, returning the physical address of the
    * segment allocated in physical_location if physical_location != NULL.
    * @param page_align align returned address on a page boundary.
    * @param[out] physical_location return physical address of the allocated space.
    */
    void* allocate(uint32_t size, bool page_align = false, address_t* physical_location = NULL);

    /*!
    * Deallocate the memory allocated to p.
    */
    void free(void* p);

    /*!
    * Changes the size of the memory block pointed to by @c ptr to @c size bytes.
    * The contents will be unchanged up to the minimum of the old and new sizes;
    * newly allocated memory will be uninitialized. If @c ptr is NULL, then the call is
    * equivalent to allocate(size), for all values of size; if @c size is equal to zero,
    * and @c ptr is not NULL, then the call is equivalent to free(ptr).
    * Unless ptr is NULL, it must have been returned by an earlier call to allocate()
    * or reallocate(). If the area pointed to was moved, a free(ptr) is done.
    */
    void* reallocate(void* ptr, size_t size);

    /*!
    * Accessor functions for heapInitialised and placementAddress.
    */
    bool is_heap_initialized() { return heap_initialized; }
    address_t get_placement_address() { return placement_address; }
    void set_placement_address(address_t a) { placement_address = a; }

    /*!
    * Forces the placementAddress variable to be PAGE_SIZE aligned.
    */
    void align_placement_address();

    /*!
    * Accessor functions for kernel_directory, current_directory
    */
    inline page_directory_t* get_kernel_directory()  { return &kernel_directory; }
    inline page_directory_t* get_current_directory() { return current_directory; }
    void set_current_directory(page_directory_t* p)  { current_directory = p; }

    /*!
    * Causes the given range of virtual memory to get allocated physical
    * memory.
    */
    void allocate_range(address_t start_address, uint32_t size);

    /*!
    * Returns the size of the heap. For analysis purposes.
    */
    uint32_t get_heap_size();

    /*!
    * Checks the heap for integrity.
    */
    void check_integrity();

    frame_allocator_t& page_frame_allocator() { return *frame_allocator; }

private:
    /*!
     * Pointer to the "master" page directory. This holds page table pointers for kernel
     * space. All other page directories must match the entries in here to maintain easy
     * consistency of kernel-space over memory spaces.
     */
    nucleus_page_directory_t kernel_directory;

    /*!
     * Has the kernel heap been initialised yet?
     */
    bool heap_initialized;

    /*!
     * The kernel heap
     */
    heap_t heap;

    /*!
     * The page frame allocator.
     */
    /*nucleus_*/frame_allocator_t* frame_allocator;//FIXME: stack_frame_allocator_t?

    /*!
     * Before the heap is initialised, this holds the next available location
     * for 'placement new' to be called on.
     */
    address_t placement_address;

    /*!
     * The currently active page directory
     */
    page_directory_t* current_directory;
};

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
