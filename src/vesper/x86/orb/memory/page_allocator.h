//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "types.h"

class page;

class page_allocator_impl
{
public:
    virtual void alloc_frame(page* p, bool is_kernel, bool is_writeable) = 0;
    virtual void free_frame(page* p) = 0;
    virtual address_t alloc_frame() = 0;
    virtual void free_frame(address_t frame) = 0;
};

// Page allocator interface.
//
// Allocate frames of physical memory for requested pages.

class page_allocator
{
public:
    page_allocator();
    ~page_allocator();

    /**
    * Finds a free frame (swaps out if necessary) and allocates it to p.
    **/
    inline void alloc_frame(page* p, bool is_kernel = true, bool is_writeable = true)
    {
        impl->alloc_frame(p, is_kernel, is_writeable);
    }

    /**
    * Finds a free frame and returns it.
    **/
    inline address_t alloc_frame()
    {
        return impl->alloc_frame();
    }

    /**
    * Removes the frame under p's control and returns it to the pool.
    **/
    inline void free_frame(page* p)
    {
        impl->free_frame(p);
    }

    /**
    * Adds the previously allocated frame 'frame' and returns it to the pool.
    **/
    inline void free_frame(address_t frame)
    {
        impl->free_frame(frame);
    }

private:
    page_allocator_impl *impl;
};


// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
