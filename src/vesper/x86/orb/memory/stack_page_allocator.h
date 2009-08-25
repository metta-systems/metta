#pragma once

#include "page_allocator.h"

//! Stack-based page allocator implementation.
/*!
* Simple, because it doesn't do page coloring or use any other advanced techniques yet.
*/
class stack_page_frame_allocator_t : public page_frame_allocator_impl_t
{
public:
    stack_page_frame_allocator_t(address_t mem_end, multiboot_t::header_t::memmap_t* mmap);

    virtual void alloc_frame(page* p, bool is_kernel, bool is_writeable);
    virtual void free_frame(page* p);
    virtual address_t alloc_frame();
    virtual void free_frame(address_t frame);

private:
    address_t* top;   // Top of stack, this is where we get new frame from. This is physical address.
};
