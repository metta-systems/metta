#pragma once

#include "types.h"

// Excerpted from "memory_manager-arch.h"
const size_t PAGE_SIZE = 0x1000;
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

// Bootloader micro PMM allocator.
extern address_t pmm_alloc_next_page();
extern address_t pmm_alloc_page(address_t vaddr);
