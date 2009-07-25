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
// Defined in pmm.cpp.
class boot_pmm_allocator
{
public:
    boot_pmm_allocator() : alloced_start(0) {}

    void setup_pagetables();
    void adjust_alloced_start(address_t new_start);
    address_t get_alloced_start();

    address_t alloc_next_page();
    address_t alloc_page(address_t vaddr);

    void mapping_enter(address_t vaddr, address_t paddr);
    bool mapping_entered(address_t vaddr);
    void start_paging();

private:
    //! Helper to select either low or high pagetable depending on address.
    address_t *select_pagetable(address_t vaddr);

    //! Start (FIXME: and end?) of allocated pages for passing into initcp.
    address_t alloced_start;

    address_t *kernelpagedir;
    address_t *lowpagetable;
    address_t *highpagetable;
};
