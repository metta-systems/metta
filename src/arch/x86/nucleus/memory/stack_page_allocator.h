//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "frame_allocator.h"
#include "lockable.h"

//! Stack-based page allocator implementation.
/*!
 * Simple, because it doesn't do page coloring or use any other advanced techniques yet.
 */
class stack_frame_allocator_t : public frame_allocator_t, public lockable_t
{
public:
    stack_frame_allocator_t();

    virtual void init(multiboot_t::mmap_t* mmap, page_directory_t* pd);
    virtual address_t alloc_frame();
    virtual address_t alloc_frame(address_t virt);
    virtual void free_frame(address_t frame, address_t virt = 0);

private:
    void free_frame_internal(address_t frame, address_t mapping);

    address_t  next_free_phys; //!< Top of the stack, this is where we get new frame from. This is physical address.
    uint32_t   total_frames;
    uint32_t   free_frames;
    uint32_t   reserved_frames;
    page_directory_t* pagedir;
};

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
