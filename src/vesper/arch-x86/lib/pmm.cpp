//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
/*!
* Minimal paged memory allocator.
*
* Used for startup and paged mode initialization.
* Uses new/delete from c++boot.cpp
*/
#include "memory.h"
#include "default_console.h"
#include "minmax.h"
#include "mmu.h"
#include "memutils.h"

#define BOOT_PMM_DEBUG 0

namespace kickstart_n
{

memory_allocator_t::memory_allocator_t()
    : pagedir()
    , alloc_start(0)
{
    // TODO: map to top of memory (requires keeping track of pde/pte physical addresses)
//     mapping_enter((address_t)pagedir, (address_t)pagedir);
}

void memory_allocator_t::start_paging()
{
    ia32_mmu_t::set_active_pagetable(pagedir.get_physical());
    ia32_mmu_t::enable_paged_mode();

#if BOOT_PMM_DEBUG
    kconsole << WHITE << "Enabled paging." << endl;
#endif
}

void memory_allocator_t::adjust_alloc_start(address_t new_start)
{
    alloc_start = max(alloc_start, new_start);
    alloc_start = page_align_up<address_t>(alloc_start);
}

address_t memory_allocator_t::get_alloc_start()
{
    return alloc_start;
}

address_t memory_allocator_t::alloc_next_page()
{
    address_t ret = alloc_start;
    alloc_start += PAGE_SIZE;
    return ret;
}

address_t memory_allocator_t::alloc_page(address_t vaddr)
{
    address_t ret = alloc_next_page();
    pagedir.create_mapping(vaddr, ret);
    return ret;
}

} // namespace kickstart_n

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
