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
#include "config.h"

//======================================================================================================================
// page_table_t
//======================================================================================================================

page_table_t* page_table_t::clone(address_t* phys)
{
    page_table_t* table = new(phys) page_table_t;
    for(int i = 0; i < 1024; i++)
    {
        if (pages[i].frame())
        {
            address_t* p = 0;
            new(p) page_table_t; // use available frame allocator to get a frame for page_table

            table->pages[i].set_frame(*p);
            table->pages[i].set_present(pages[i].present());
            table->pages[i].set_writable(pages[i].writable());
            table->pages[i].set_user(pages[i].user());
            table->pages[i].set_accessed(pages[i].accessed());
            table->pages[i].set_dirty(pages[i].dirty());

            copy_frame(pages[i].frame(), table->pages[i].frame());
        }
    }
    return table;
}

void page_table_t::copy_frame(uint32_t from_phys, uint32_t to_phys)
{
    asm volatile ("pushf\n"     // Push EFLAGS, so we can pop it and reenable interrupts later, if they were enabled anyway.
    "cli\n"                     // Disable interrupts, so we aren't interrupted.

    "movl %%cr0, %%eax\n"       // Get the control register...
    "andl $0x7fffffff, %%eax\n" // ...and...
    "movl %%eax, %%cr0\n"       // ...disable paging.

    "movl $1024, %%ecx\n"       // 1024*4bytes = 4096 bytes to copy.
    "rep\n"
    "movsl\n"                   // Copy physical frame data

    "movl %%cr0, %%eax\n"       // Get the control register again...
    "orl  $0x80000000, %%eax\n" // ...and...
    "movl %%eax, %%cr0\n"       // ...enable paging.

    "popf\n"                    // Pop EFLAGS back.
    :: "S"(from_phys), "D"(to_phys) : "eax", "ecx");
}

//======================================================================================================================
// page_directory_t
//======================================================================================================================

page_directory_t::page_directory_t()
{
    for (int i = 0; i < 1023; i++)
        tables[i] = 0;
    tables[1023] = (address_t)tables | IA32_PAGE_WRITABLE | IA32_PAGE_PRESENT; //RPD trick
}

bool page_directory_t::create_mapping(address_t virt, address_t phys, int flags)
{
    page_table_t* page_table = get_page_table(virt, true);
    ASSERT(page_table);

    uint32_t pte = pte_entry(virt);
    if (!page_table->get_page(pte)->present())
    {
        // page isn't mapped
        (*page_table)[pte] = (phys & PAGE_MASK) | IA32_PAGE_PRESENT | (flags & ~PAGE_MASK);
    }
    else
    {
        // page is already mapped
        return false;
    }

#if MEMORY_DEBUG
    kconsole << "+(" << virt << "=>" << phys << ") to entry " << pte << " @" << (address_t)page_table << "(" << (address_t)page_table->get_page(pte) << ")" << endl;
#endif

    return true;
}

void page_directory_t::remove_mapping(address_t virt)
{
    page_table_t* page_table = get_page_table(virt, false);

    if (page_table)
    {
        uint32_t pde = pde_entry(virt);
        uint32_t pte = pte_entry(virt);
        int i;

        if (page_table->get_page(pte)->present())
        {
            // page is mapped, so unmap it
            (*page_table)[pte] = IA32_PAGE_WRITABLE; // r/w, not present
        }

        // check if there are any more present PTEs in this page table
        for (i = 0; i < 1024; i++)
        {
            if(page_table->get_page(i)->present())
                break;
        }

        // if there are none, then free the space allocated to the page table and delete mappings
        if (i == 1024)
        {
            page_table_t* pt = reinterpret_cast<page_table_t*>(tables[pde] & PAGE_MASK);
            delete pt;
            tables[pde] = IA32_PAGE_WRITABLE;
        }
    }
}

bool page_directory_t::mapping_exists(address_t virt)
{
    page_table_t* page_table = get_page_table(virt, false);
    if (!page_table)
        return false;

    uint32_t pte = pte_entry(virt);
    if (!page_table->get_page(pte)->present())
        return false;

    return true;
}

page_t* page_directory_t::get_page(address_t virt)
{
    page_table_t* page_table = get_page_table(virt, false);
    if (!page_table)
        return 0;

    uint32_t pte = pte_entry(virt);
    if (!page_table->get_page(pte)->present())
        return 0;

    return page_table->get_page(pte);
}

void page_directory_t::dump()
{
    kconsole << WHITE << "Dumping page directory:" << endl;
    for (int i = 0; i < 1024; i++)
    {
        if(!(tables[i] & IA32_PAGE_PRESENT))
            continue;

        page_table_t* pagetable = get_page_table(i << PDE_SHIFT, false);
        ASSERT(pagetable);

        kconsole << "  Page table " << i << endl;
        for (int k = 0; k < 1024; k++)
        {
            page_t* page = pagetable->get_page(k);

            if (page->present())
                kconsole << "    " << (unsigned int)(i << PDE_SHIFT) + (k << PTE_SHIFT) << " => " << page->frame() << endl;
        }

        kconsole << "  End of table." << endl;
    }
}

// page_directory_t* page_directory_t::clone()
// {
//     address_t phys;
//     page_directory_t* dir = new(true, &phys) page_directory_t;
// 
//     // Get the offset of tables_physical from the start of
//     // the page_directory object.
//     uint32_t offset = (address_t)dir->tables_physical - (address_t)dir;
// 
//     // Then the physical address of dir->tables_physical is
//     dir->physical_address = phys + offset;
// 
//     // Go through each page table. If the page table is in the kernel directory, do not make a new copy.
//     for (int i = 0; i < 1024; i++)
//     {
//         if (!tables[i])
//             continue;
// 
// //FIXME: requires knowledge of nucleus internals, candidate for vm_server method?
// //         if (nucleus.mem_mgr().get_kernel_directory()->get_table(i) == tables[i])
// //         {
// // It's in the kernel, so just use the same pointer.
// //             dir->tables[i] = tables[i];
// //             dir->tables_physical[i] = tables_physical[i];
// //         }
// //         else
//             {
//                 // copy the table.
//                 address_t phys;
//                 dir->tables[i] = tables[i]->clone(&phys);
//                 dir->tables_physical[i] = phys | IA32_PAGE_PRESENT | IA32_PAGE_WRITABLE | IA32_PAGE_USER;
//             }
//     }
// 
//     return dir;
// }

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
