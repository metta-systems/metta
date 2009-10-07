//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "default_console.h"
#include "page_directory.h"
#include "memory.h"
#include "memutils.h"
#include "ia32.h"
#include "mmu.h"
#include "config.h"

page_directory_t::page_directory_t()
{
    for (int i = 0; i < 1023; i++)
        tables[i] = 0;
    tables[1023] = (address_t)tables | IA32_PAGE_WRITABLE | IA32_PAGE_PRESENT; //RPD trick
}

page_t* page_directory_t::create_mapping(address_t virt, address_t phys)
{
    page_table_t* pt = page_table(virt, true);
    ASSERT(pt);

    uint32_t pte = pte_entry(virt);
    if (!pt->page(pte).present()) // page isn't mapped
    {
        pt->page(pte) = (phys & PAGE_MASK) | IA32_PAGE_WRITABLE | IA32_PAGE_PRESENT;
        ia32_mmu_t::flush_page_directory_entry(virt); // or leave it to client?
    }
    else
    {
        // page is already mapped
        return 0;
    }

#if MEMORY_DEBUG
    kconsole << "+(" << virt << "=>" << phys << ") to entry " << pte << " @" << (address_t)pt << "(" << (address_t)&pt->page(pte) << ")" << endl;
#endif

    return &pt->page(pte);
}

void page_directory_t::remove_mapping(address_t virt)
{
    page_table_t* pt = page_table(virt, false);

    if (pt)
    {
        uint32_t pte = pte_entry(virt);

        if (pt->page(pte).present())
        {
            // page is mapped, so unmap it
            pt->page(pte) = IA32_PAGE_WRITABLE; // r/w, not present
            ia32_mmu_t::flush_page_directory_entry(virt);
        }
    }
}

bool page_directory_t::mapping_exists(address_t virt)
{
    page_table_t* pt = page_table(virt, false);
    if (!pt)
        return false;

    uint32_t pte = pte_entry(virt);
    if (!pt->page(pte).present())
        return false;

    return true;
}

page_t* page_directory_t::mapping(address_t virt, bool make)
{
    page_table_t* pt = page_table(virt, make);
    if (!pt)
        return 0;

    uint32_t pte = pte_entry(virt);
    if (!pt->page(pte).present() && !make)
        return 0;

    return &pt->page(pte);
}

void page_directory_t::dump()
{
    kconsole << WHITE << "Dumping page directory:" << endl;
    for (int i = 0; i < 1024; i++)
    {
        if(!(tables[i] & IA32_PAGE_PRESENT))
            continue;

        page_table_t* pt = page_table(i << PDE_SHIFT, false);
        ASSERT(pt);

        kconsole << "  Page table " << i << endl;
        for (int k = 0; k < 1024; k++)
        {
            page_t& page = pt->page(k);

            if (page.present())
                kconsole << "    " << (unsigned int)(i << PDE_SHIFT) + (k << PTE_SHIFT) << " => " << page.frame() << endl;
        }

        kconsole << "  End of table." << endl;
    }
    kconsole << "End of directory." << endl;
}

void page_directory_t::set_page_table(address_t virt, address_t phys)
{
    tables[pde_entry(virt)] = (phys & PAGE_MASK) | IA32_PAGE_WRITABLE | IA32_PAGE_PRESENT;
    ia32_mmu_t::flush_page_directory_entry(virt);
}

// bool page_directory_t::reclaim_pagetable(page_table_t* page_table)
// {
//         uint32_t pde = pde_entry(virt);
//         int i;
//     // check if there are any more present PTEs in this page table
//     for (i = 0; i < 1024; i++)
//     {
//         if(page_table->get_page(i)->present())
//             break;
//     }
//
//     // if there are none, then free the space allocated to the page table and delete mappings
//     if (i == 1024)
//     {
//         page_table_t* pt = reinterpret_cast<page_table_t*>(tables[pde] & PAGE_MASK);
//         delete pt;
//         tables[pde] = IA32_PAGE_WRITABLE;
//     }
// }

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
