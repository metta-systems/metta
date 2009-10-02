//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "page_allocator.h"
#include "lockable.h"

//! Stack-based page allocator implementation.
/*!
* Simple, because it doesn't do page coloring or use any other advanced techniques yet.
*/
class stack_page_frame_allocator_t : public page_frame_allocator_impl_t, public lockable_t
{
public:
    stack_page_frame_allocator_t();

    virtual void init(multiboot_t::mmap_t* mmap, page_directory_t* pd);
    virtual void alloc_frame(page_t* p, bool is_kernel, bool is_writeable);
    virtual void free_frame(page_t* p);
    virtual address_t alloc_frame();
    virtual void free_frame(address_t frame);

private:
    address_t  next_free_phys; //!< Top of the stack, this is where we get new frame from. This is physical address.
    uint32_t   total_frames;
    uint32_t   free_frames;
    uint32_t   reserved_frames;
    page_directory_t* pagedir;
};

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
