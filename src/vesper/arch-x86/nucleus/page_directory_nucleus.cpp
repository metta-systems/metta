//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "default_console.h"
#include "page_directory.h"
#include "memory/memory_manager.h" // for RPAGETAB_VBASE
#include "memory.h"
#include "memutils.h"
#include "ia32.h"
#include "mmu.h"
#include "nucleus.h"
#include "config.h"

using nucleus_n::nucleus;

/*!
 * Abstracted away process of getting pointers to page tables.
 *
 * Nucleus version via recursive pagedir.
 */
page_table_t* page_directory_t::page_table(address_t virt, bool make)
{
    uint32_t pde = pde_entry(virt);
    page_table_t* page_table = 0;

    if (tables[pde] & IA32_PAGE_PRESENT)
    {
        // page table exists.
        page_table = (page_table_t*)(RPAGETAB_VBASE + (pde * PAGE_SIZE));
    }
    else if (make) // doesn't exist, so alloc a page and add into pdir
    {
        address_t new_phys = nucleus.mem_mgr().page_frame_allocator().alloc_frame();
#if MEMORY_DEBUG
        kconsole << "allocating new pagetable " << pde << " @ " << new_phys << endl;
#endif
        page_table = (page_table_t*)(RPAGETAB_VBASE + (pde * PAGE_SIZE));

        tables[pde] = (new_phys & PAGE_MASK) | IA32_PAGE_WRITABLE | IA32_PAGE_PRESENT;
        ia32_mmu_t::flush_page_directory_entry(virt);
        page_table->zero();
    }

    return page_table;
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
