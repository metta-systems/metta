#include "page_directory.h"

//======================================================================================================================
// page_table_t
//======================================================================================================================

page_table_t* page_table_t::clone(address_t* phys)
{
    page_table* table = new(true, phys) page_table_t();
    for(int i = 0; i < 1024; i++)
    {
        if (pages[i].frame())
        {
            kmemmgr.alloc_frame(&table->pages[i]);//!!!
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
    uint32_t dummy;
    asm volatile ("pushf\n"     // push EFLAGS, so we can pop it and reenable interrupts later, if they were enabled anyway.
    "cli\n"                     // Disable interrupts, so we aren't interrupted.

    "movl %%cr0, %0\n"          // Get the control register...
    "andl $0x7fffffff, %0\n"    // and...
    "movl %0, %%cr0\n"          // Disable paging.

    "mov ecx, 1024\n"           // 1024*4bytes = 4096 bytes
    "rep\n"
    "movsl\n"

    "movl %%cr0, %0\n"          // Get the control register again
    "orl  $0x80000000, %0\n"    // and...
    "movl %0, %%cr0\n"          // Enable paging.

    "popf\n"                    // Pop EFLAGS back.
    :: "r"(dummy), "S"(from_phys), "D"(to_phys));
}

//======================================================================================================================
// page_directory_t
//======================================================================================================================

page_directory_t::page_directory_t()
{
    for (int i = 0; i < 1024; i++)
    {
        tables[i] = 0;
        tables_physical[i] = 0;
    }
    physical_addr = (address_t)tables_physical;
}

page_t* page_directory_t::get_page(address_t addr, bool make)
{
    addr /= PAGE_SIZE;
    uint32_t table_idx = addr / 1024;
    if (tables[table_idx]) // if the table is already assigned
    {
        return tables[table_idx]->get_page(addr % 1024);
    }
    else if (make)
    {
        address_t tmp;
        tables[table_idx] = new(true/*page aligned*/, &tmp/*give phys. addr*/) page_table_t();
        tables_physical[table_idx] = tmp | 0x7; // PRESENT, RW, US
        return tables[table_idx]->get_page(addr % 1024);
    }
    else
    {
        return NULL;
    }
}

page_directory_t* page_directory_t::clone()
{
    address_t phys;
    page_directory *dir = new(true, &phys) page_directory_t();

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

        if (orb.root_pagedir()->get_table(i) == tables[i])
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
            dir->tables_physical[i] = phys | 0x07;
        }
    }

    return dir;
}

void page_directory_t::dump()
{
    bool b = false;
    kconsole << WHITE << "Dumping page directory:" << endl;
    for (int i = 0; i < 0xFFFFF; i++)
    {
        page_t* page;
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
