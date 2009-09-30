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

const size_t    PAGE_SIZE = 0x1000;
const address_t PAGE_MASK = 0xFFFFF000;

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

// Primitive page frame allocator for bootstrap.
// Defined in pmm.cpp.
class memory_allocator_t
{
public:
    memory_allocator_t();
    page_directory_t& root_pagedir() { return pagedir; }

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
