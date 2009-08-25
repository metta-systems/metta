#include "default_console.h"
#include "page_directory.h"
#include "memory/new.h"
#include "nucleus.h"
#include "ia32.h"

using nucleus::orb;

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
            orb.mem_mgr().page_frame_allocator().alloc_frame(&table->pages[i]);

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
    for (int i = 0; i < 1024; i++)
    {
        tables[i] = 0;
        tables_physical[i] = 0;
    }
    physical_address = (address_t)tables_physical;
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
        address_t phys;
        tables[table_idx] = new(true, &phys) page_table_t();
        tables_physical[table_idx] = phys | IA32_PAGE_PRESENT | IA32_PAGE_WRITABLE | IA32_PAGE_USER;
        return tables[table_idx]->get_page(addr % 1024);
    }

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

        if (orb.mem_mgr().get_kernel_directory()->get_table(i) == tables[i])
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
