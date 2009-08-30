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
*/
#include "memory.h"
#include "default_console.h"
#include "minmax.h"
#include "mmu.h"
#include "memutils.h"
#include "ia32.h"
#include "page_directory.h"

#define BOOT_PMM_DEBUG 0

#define PDE_SHIFT 22
#define PDE_MASK  0x3ff
#define PTE_SHIFT 12
#define PTE_MASK  0x3ff

inline int pde_entry(address_t vaddr)
{
    return (vaddr >> PDE_SHIFT) & PDE_MASK;
}

inline int pte_entry(address_t vaddr)
{
    return (vaddr >> PTE_SHIFT) & PTE_MASK;
}

void boot_pmm_allocator::setup_pagetables()
{
    // Create and configure paging directory.
    kernelpagedir = reinterpret_cast<address_t*>(alloc_next_page());
    memutils::fill_memory(kernelpagedir, 0, PAGE_SIZE);

    // TODO: map to top of memory (requires keeping track of pde/pte physical addresses)
    mapping_enter((address_t)kernelpagedir, (address_t)kernelpagedir);
}

void boot_pmm_allocator::start_paging()
{
    ia32_mmu_t::set_active_pagetable((address_t)kernelpagedir);
    ia32_mmu_t::enable_paged_mode();

#if BOOT_PMM_DEBUG
    kconsole << "Enabled paging." << endl;
#endif
}

void boot_pmm_allocator::adjust_alloced_start(address_t new_start)
{
    alloced_start = max(alloced_start, new_start);
    alloced_start = page_align_up<address_t>(alloced_start);
}

address_t boot_pmm_allocator::get_alloced_start()
{
    return alloced_start;
}

page_table_t* boot_pmm_allocator::select_pagetable(address_t vaddr)
{
    page_table_t* pagetable = 0;
    if (kernelpagedir[pde_entry(vaddr)])
        pagetable = reinterpret_cast<page_table_t*>(kernelpagedir[pde_entry(vaddr)] & PAGE_MASK);
    else
    {
        pagetable = reinterpret_cast<page_table_t*>(alloc_next_page());
        memutils::fill_memory(pagetable, 0, PAGE_SIZE);
        kernelpagedir[pde_entry(vaddr)] = (address_t)pagetable | IA32_PAGE_PRESENT | IA32_PAGE_WRITABLE;
    }

    return pagetable;
}

void boot_pmm_allocator::mapping_enter(address_t vaddr, address_t paddr)
{
    page_table_t *pagetable = select_pagetable(vaddr);
    ASSERT(pagetable);
#if BOOT_PMM_DEBUG
    kconsole << "+(" << vaddr << "=>" << paddr << ") to entry " << pte_entry(vaddr) << " @" << (address_t)pagetable << "(" << (address_t)&((*pagetable)[pte_entry(vaddr)]) << ")" << endl;
#endif
    (*pagetable)[pte_entry(vaddr)] = paddr | IA32_PAGE_PRESENT | IA32_PAGE_WRITABLE;

    if (!mapping_entered((address_t)pagetable))
        mapping_enter((address_t)pagetable, (address_t)pagetable);
}

bool boot_pmm_allocator::mapping_entered(address_t vaddr)
{
    page_table_t *pagetable = select_pagetable(vaddr);
    ASSERT(pagetable);
    return (*pagetable)[pte_entry(vaddr)] != 0;
}

// TODO: pmm interface + pmm_state transfer

address_t boot_pmm_allocator::alloc_next_page()
{
    address_t ret = alloced_start;
    alloced_start += PAGE_SIZE;
    return ret;
}

address_t boot_pmm_allocator::alloc_page(address_t vaddr)
{
    address_t ret = alloc_next_page();
    mapping_enter(vaddr, ret);
    return ret;
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
