//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "types.h"
#include "ia32.h"
#include "page_directory.h"
#include "frame_allocator.h"

template <typename T>
inline T page_align_up(T a)
{
    if (a % PAGE_SIZE)
    {
        a &= PAGE_MASK;
        a += PAGE_SIZE;
    }
    return a;
}

template <typename T>
inline T page_align_down(T a)
{
    return a - a % PAGE_SIZE;
}

template <typename T>
inline T page_align_down(void* a)
{
    const T b = reinterpret_cast<T>(a);
    return b - b % PAGE_SIZE;
}

template <typename T>
inline bool is_page_aligned(T a)
{
    return a % PAGE_SIZE == 0;
}

namespace kickstart_n {

class kickstart_frame_allocator_t : public page_frame_allocator_impl_t
{
public:
    virtual void init(multiboot_t::mmap_t* mmap, page_directory_t* pd);
    virtual void alloc_frame(page_t* p, bool is_kernel, bool is_writeable);
    virtual void free_frame(page_t* p);
    virtual address_t alloc_frame();
    virtual void free_frame(address_t frame);
};

// Primitive page frame allocator for bootstrap.
// Defined in pmm.cpp.
class memory_allocator_t
{
public:
    memory_allocator_t();
    page_directory_t& root_pagedir() { return pagedir; }
    frame_allocator_t* frame_allocator();

    void adjust_alloc_start(address_t new_start);
    address_t get_alloc_start();

    address_t alloc_next_page();
    address_t alloc_page(address_t vaddr);

    void start_paging();

private:
    page_directory_t pagedir;
    address_t alloc_start;
};
//different new impls will allocate data for page_directory_t differently, so alloc strategies
// inside kickstart and in nucleus will differ, while structures used will be the same.
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
