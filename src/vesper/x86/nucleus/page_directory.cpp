//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "default_console.h"
#include "page_directory.h"
#include "memory/new.h"
#include "nucleus.h"
#include "ia32.h"

using nucleus_n::nucleus;

//======================================================================================================================
// page_table_t
//======================================================================================================================

page_table_t* page_table_t::clone(address_t* phys)
{
    page_table_t* table = new(true, phys) page_table_t();
    for(int i = 0; i < 1024; i++)
    {
        if (pages[i].frame())
        {
            nucleus.vm_server().page_frame_allocator().alloc_frame(&table->pages[i]);

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

page_directory_t::page_directory_t()
{
    for (int i = 0; i < 1024; i++)
    {
        tables[i] = 0;
        tables_physical[i] = 0;
    }
    physical_address = (address_t)tables_physical;
}

page_table_t* page_directory_t::get_page_table(address_t vaddr, bool make)
{
    uint32_t table_idx = pde_entry(vaddr);
    if (!tables[table_idx] && make)
    {
        address_t phys;
        tables[table_idx] = new(true, &phys) page_table_t();
        tables_physical[table_idx] = phys | IA32_PAGE_PRESENT | IA32_PAGE_WRITABLE | IA32_PAGE_USER;
    }
    return tables[table_idx];
}

page_t* page_directory_t::get_page(address_t addr, bool make)
{
    page_table_t* pagetable = get_page_table(addr, make);

    if (pagetable)
        return pagetable->get_page(pte_entry(addr));

    return NULL;
}

page_directory_t* page_directory_t::clone()
{
    address_t phys;
    page_directory_t* dir = new(true, &phys) page_directory_t();

    // Get the offset of tables_physical from the start of
    // the page_directory object.
    uint32_t offset = (address_t)dir->tables_physical - (address_t)dir;

    // Then the physical address of dir->tables_physical is
    dir->physical_address = phys + offset;

    // Go through each page table. If the page table is in the kernel directory, do not make a new copy.
    for (int i = 0; i < 1024; i++)
    {
        if (!tables[i])
            continue;

        if (nucleus.mem_mgr().get_kernel_directory()->get_table(i) == tables[i])
        {
            // It's in the kernel, so just use the same pointer.
            dir->tables[i] = tables[i];
            dir->tables_physical[i] = tables_physical[i];
        }
        else
        {
            // copy the table.
            address_t phys;
            dir->tables[i] = tables[i]->clone(&phys);
            dir->tables_physical[i] = phys | IA32_PAGE_PRESENT | IA32_PAGE_WRITABLE | IA32_PAGE_USER;
        }
    }

    return dir;
}

void page_directory_t::enter_mapping(address_t vaddr, address_t paddr, int flags)
{
    page_table_t* pagetable = get_page_table(vaddr, true);
    ASSERT(pagetable);
#if MEMORY_DEBUG
    kconsole << "+(" << vaddr << "=>" << paddr << ") to entry " << pte_entry(vaddr) << " @" << (address_t)pagetable << "(" << (address_t)&((*pagetable)[pte_entry(vaddr)]) << ")" << endl;
#endif
    (*pagetable)[pte_entry(vaddr)] = paddr | IA32_PAGE_PRESENT | (flags & ~PAGE_MASK);

//     if (!mapping_entered((address_t)pagetable))
//         mapping_enter((address_t)pagetable, (address_t)pagetable);
}

void page_directory_t::dump()
{
    bool b = false;
    kconsole << WHITE << "Dumping page directory:" << endl;
    for (int i = 0; i < 0xFFFFF; i++)
    {
        page_t* page = 0;
        if (tables[i / 1024])
            page = tables[i / 1024]->get_page(i % 1024);
        else if (b)
        {
            kconsole << "    End of table." << endl;
            b = false;
        }
        else
            continue;

        if (!b && page->present())
        {
            b = true;
            kconsole << "    " << (unsigned int)(i << 12) << endl;
        }
        else
            if(b && !page->present())
            {
                b = false;
                kconsole << "    " << (unsigned int)(i << 12) << endl;
            }
    }
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
