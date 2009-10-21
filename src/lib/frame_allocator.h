//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
//
// Physical memory frame allocator interface.
//
#pragma once

#include "types.h"
#include "multiboot.h"

class page_t;
class page_directory_t;

//! Allocate frames of physical memory for requested pages.
class frame_allocator_t
{
public:
    /*!
     * Initialize free physical memory map.
     */
    virtual void init(multiboot_t::mmap_t* mmap, page_directory_t* pd) = 0;

    /*!
     * Finds a free frame and returns it.
     */
    virtual address_t alloc_frame() = 0;

    /*!
     * Finds a free frame, enters it into page mapping to address @p virt and returns it.
     */
    virtual address_t alloc_frame(address_t virt) = 0;

    /*!
     * Frees the previously allocated frame 'frame' and returns it to the pool.
     * If @e virt is non-zero, remove mapping associated with address @e virt in frame allocator's pagedir.
     */
    virtual void free_frame(address_t frame, address_t virt = 0) = 0;

    /*!
    * Finds a free frame (swaps out if necessary) and allocates it to p.
    */
    void alloc_frame(page_t* p, bool is_kernel = true, bool is_writeable = true);

    /*!
    * Removes the frame under p's control and returns it to the pool.
    */
    void free_frame(page_t* p);
};

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
