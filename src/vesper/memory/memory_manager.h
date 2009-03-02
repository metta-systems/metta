//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "types.h"
#include "heap.h"
#include "multiboot.h"
#include "bit_array.h"

namespace metta {
namespace kernel {

#define HEAP_START              0xE0000000
#define HEAP_INITIAL_SIZE       0x100000
#define HEAP_END                0xEFFFFFFF
// #define USER_HEAP_START         0xD0000000
// #define USER_HEAP_INITIAL_SIZE  0x100000
// #define USER_HEAP_END           0xDFFFFFFF
#define STACK_START             (0xF8000000-0x4)
#define STACK_INITIAL_SIZE      0x10000
#define STACK_END               0xF0000000
#define KERNEL_START            0xC0000000

#define STACK_ADDRESS(x)     ((address_t)x <= STACK_START && (address_t)x > STACK_END)
#define HEAP_ADDRESS(x)      ((address_t)x >= HEAP_START  && (address_t)x < USER_HEAP_START)
#define USER_HEAP_ADDRESS(x) ((address_t)x >= USER_HEAP_START && (address_t)x <= USER_HEAP_END)

class page;
class page_directory;
using metta::common::bit_array;

#define kmemmgr memory_manager::self()

/**
* Handles all memory related events. Heap startup, allocation, deallocation,
* virtual memory etc.
**/
class memory_manager
{
public:
    /**
    * Heap can call our private alloc_frame() functions.
    **/
    friend class heap;

    /**
    * Singleton accessor.
    **/
    static memory_manager& self();

    /**
    * Normal constructor - passes the address of end of memory.
    * Initialises paging and sets up a standard kernel page directory.
    * Enables paging, then maps some pages for the heap.
    **/
    void init(address_t mem_end, multiboot::header::memmap *mmap);

    /**
    * Allocate "size" bytes, returning the physical address of the
    * segment allocated in physical_location if physical_location != NULL.
    **/
    void* malloc(uint32_t size, bool page_align = false,
                 address_t* physical_location = NULL);

    /**
    * Deallocate the memory allocated to p.
    **/
    void free(void* p);

    /**
    * Changes the size of the memory block pointed to by ptr to size bytes.
    * The contents will be unchanged to the minimum of the old and new sizes;
    * newly allocated memory will be uninitialized. If ptr is NULL, then the call is
    * equivalent to malloc(size), for all values of size; if size is equal to zero,
    * and ptr is not NULL, then the call is equivalent to  free(ptr).
    * Unless ptr is NULL, it must have been returned by an earlier call to malloc()
    * or realloc(). If the area pointed to was moved, a free(ptr) is done.
    **/
    void *realloc(void *ptr, size_t size);

    /**
    * Allocate "size" bytes from the *user space heap*.
    **/
    void* umalloc(uint32_t size);

    /**
    * Deallocate any memory allocated to p via umalloc.
    **/
    void ufree(void* p);

    /**
    * Accessor functions for heapInitialised and placementAddress.
    **/
    bool is_heap_initialised() { return heap_initialised; }
    address_t get_placement_address() { return placement_address; }
    void set_placement_address(address_t a) { placement_address = a; }

    /**
    * Forces the placementAddress variable to be PAGE_SIZE aligned.
    **/
    void align_placement_address();

    /**
    * Changes the original stack given by the bootloader to one at
    * a virtual memory location defined at compile time.
    **/
    void remap_stack();

    /**
    * Accessor functions for kernel_directory, current_directory
    **/
    inline page_directory* get_kernel_directory()  { return kernel_directory; }
    inline page_directory* get_current_directory() { return current_directory; }
    void set_current_directory(page_directory* p) { current_directory = p; }

    /**
    * Finds a free frame (swaps out if necessary) and allocates it to p.
    **/
    void alloc_frame(page* p, bool is_kernel = true, bool is_writeable = true);

    /**
    * Finds a free frame and returns it.
    **/
    address_t alloc_frame();

    /**
    * Removes the frame under p's control and returns it to the pool.
    **/
    void free_frame(page* p);

    /**
    * Adds the previously allocated frame 'frame' and returns it to the pool.
    **/
    void free_frame(address_t frame);

    /**
    * Causes the given range of virtual memory to get allocated physical
    * memory.
    **/
    void allocate_range(address_t start_address, uint32_t size);

    /**
    * Returns the size of the kernel heap. For analysis purposes.
    **/
    uint32_t get_kernel_heap_size();
    uint32_t get_user_heap_size();

    /**
    * Checks the kernel and user heap for integrity.
    **/
    void check_integrity();

private:
    /**
    * Default constructor.
    **/
    memory_manager();
//     ~memory_manager();

private:
    /**
    * Array of frames to describe physical memory state.
    **/
    bit_array *frames;

    /**
    * Total number of physical memory frames.
    **/
    uint32_t n_frames;

    /**
    * Has the kernel heap been initialised yet?
    **/
    bool heap_initialised;

    /**
    * The kernel heap
    **/
    heap heap_;

    /**
    * The user-mode shared heap
    **/
    heap user_heap;

    /**
    * Before the heap is initialised, this holds the next available location
    * for 'placement new' to be called on.
    **/
    address_t placement_address;

    /**
    * The currently active page directory
    **/
    page_directory *current_directory;

    /**
    * Pointer to the "master" page directory. This holds page table pointers for kernel
    * space. All other page directories must match the entries in here to maintain easy
    * consistency of kernel-space over memory spaces.
    **/
    page_directory *kernel_directory;
};

}
}

#include "memory_manager-arch.h"

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
