//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "default_console.h"
#include "page_directory.h"
#include "memory/new.h"
#include "memory.h"
#include "memutils.h"
#include "ia32.h"
#include "mmu.h"

/*!
 * Abstracted away process of getting pointers to page tables.
 *
 * Kickstart version via physmem pointers.
 */
page_table_t* kickstart_page_directory_t::page_table(address_t virt, bool make)
{
    uint32_t pde = pde_entry(virt);
    page_table_t* page_table = 0;

    if (directory[pde] & IA32_PAGE_PRESENT)
    {
        page_table = reinterpret_cast<page_table_t*>(directory[pde] & PAGE_MASK);
    }
    else if (make) // doesn't exist, so alloc a page and add into pdir
    {
        address_t phys;
        page_table = new(&phys) page_table_t;

        directory[pde] = (phys & PAGE_MASK) | IA32_PAGE_WRITABLE | IA32_PAGE_PRESENT;
        ia32_mmu_t::flush_page_directory_entry(virt);
        page_table->zero();
    }

    return page_table;
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
