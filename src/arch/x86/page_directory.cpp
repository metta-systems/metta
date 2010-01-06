//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus@exquance.com>
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
#include "frame.h"
#include "config.h"

void page_directory_t::init()
{
    directory_virtual = new(&directory_physical) frame_t;
    for (int i = 0; i < 1023; i++)
        directory[i] = 0;
    directory[1023] = (address_t)directory | IA32_PAGE_WRITABLE | IA32_PAGE_PRESENT; //RPD trick
}

void page_directory_t::init(uint32_t* placement_area)
{
    directory = placement_area;
    for (int i = 0; i < 1023; i++)
        directory[i] = 0;
    directory[1023] = (address_t)directory | IA32_PAGE_WRITABLE | IA32_PAGE_PRESENT; //RPD trick
}

page_t* page_directory_t::create_mapping(address_t virt, address_t phys)
{
    page_table_t* pt = page_table(virt, true);
    ASSERT(pt);

    uint32_t pte = pte_entry(virt);
    if (!pt->page(pte).present()) // page isn't mapped
    {
        pt->page(pte) = (phys & PAGE_MASK) | IA32_PAGE_WRITABLE | IA32_PAGE_PRESENT;
        ia32_mmu_t::flush_page_directory_entry(virt);
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

bool page_directory_t::is_mapped(address_t virt)
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
        if(!(directory[i] & IA32_PAGE_PRESENT))
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
    directory[pde_entry(virt)] = (phys & PAGE_MASK) | IA32_PAGE_WRITABLE | IA32_PAGE_PRESENT;
    ia32_mmu_t::flush_page_directory_entry(virt);
}

#include "memory/memory_manager.h" // for RPAGETAB_VBASE
page_table_t* page_directory_t::page_table(address_t virt, bool make)
{
    uint32_t pde = pde_entry(virt);
    page_table_t* page_table = 0;

    if (directory_access[pde] & IA32_PAGE_PRESENT)
    {
        if (directory_access == directory_physical)
            page_table = reinterpret_cast<page_table_t*>(directory[pde] & PAGE_MASK);
        else
            page_table = (page_table_t*)(RPAGETAB_VBASE + (pde * PAGE_SIZE));
    }
    else if (make) // doesn't exist, so alloc a page and add into pdir
    {
        address_t phys;
        page_table = new(&phys) page_table_t;
#if MEMORY_DEBUG
        kconsole << "allocating new pagetable " << pde << " @ " << phys << endl;
#endif

        set_page_table(virt, phys);
        page_table->zero();
    }

    return page_table;
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
//         page_table_t* pt = reinterpret_cast<page_table_t*>(directory[pde] & PAGE_MASK);
//         delete pt;
//         directory[pde] = IA32_PAGE_WRITABLE;
//     }
// }

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
