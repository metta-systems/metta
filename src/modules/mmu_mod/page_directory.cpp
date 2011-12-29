//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2011, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "page_directory.h"
#include "default_console.h"

void page_t::dump()
{
    kconsole << "page: frame " << frame() << " ";
    dump_flags();
    kconsole << endl;
}

void page_t::dump_flags()
{
#define FLAG(x) (int)((raw & (x)) != 0)
kconsole << "P:" << FLAG(IA32_PAGE_PRESENT)
         << " W:" << FLAG(IA32_PAGE_WRITABLE)
         << " U:" << FLAG(IA32_PAGE_USER)
         << " WT:" << FLAG(IA32_PAGE_WRITE_THROUGH)
         << " CD:" << FLAG(IA32_PAGE_CACHE_DISABLE)
         << " A:" << FLAG(IA32_PAGE_ACCESSED)
         << " D:" << FLAG(IA32_PAGE_DIRTY)
         << " 4:" << FLAG(IA32_PAGE_4MB)
         << " G:" << FLAG(IA32_PAGE_GLOBAL)
         << " S:" << FLAG(IA32_PAGE_SWAPPED)
         << " COW:" << FLAG(IA32_PAGE_COW);
#undef FLAG
}

// Get CPU-agnostic flags of a vm page
flags_t page_t::flags()
{
    flags_t flags = executable;
    if (raw & IA32_PAGE_WRITABLE)
        flags |= writable;
    if ((raw & IA32_PAGE_USER) == 0)
        flags |= kernel_mode;
    if (raw & IA32_PAGE_WRITE_THROUGH)
        flags |= write_through;
    if (raw & IA32_PAGE_CACHE_DISABLE)
        flags |= cache_disable;
    if (raw & IA32_PAGE_SWAPPED)
        flags |= swapped;
    if (raw & IA32_PAGE_COW)
        flags |= copy_on_write;
    if (raw & IA32_PAGE_GLOBAL)
        flags |= global;
    return flags;
}

void page_t::set_flags(flags_t flags)
{
    uint32_t value = 0;
    if (flags & kernel_mode)
        value |= IA32_PAGE_GLOBAL;
    else
        value |= IA32_PAGE_USER;
    if (flags & writable)
        value |= IA32_PAGE_WRITABLE;
    if (flags & write_through)
        value |= IA32_PAGE_WRITE_THROUGH;
    if (flags & cache_disable)
        value |= IA32_PAGE_CACHE_DISABLE;
    if (flags & swapped)
        value |= IA32_PAGE_SWAPPED;
    else
        value |= IA32_PAGE_PRESENT;
    if (flags & copy_on_write)
        value |= IA32_PAGE_COW;
    if (flags & global)
        value |= IA32_PAGE_GLOBAL;
    raw = (raw & PAGE_MASK) | value;
}

void page_t::set_4mb(bool b) // only valid in PDE
{
    if (b)
        raw = raw | IA32_PAGE_4MB;
    else
        raw = raw & ~IA32_PAGE_4MB;
}

// void page_directory_t::init()
// {
//     directory_virtual = new(&directory_physical) frame_t;
//     for (int i = 0; i < 1023; i++)
//         directory[i] = 0;
//     directory[1023] = (address_t)directory_physical | IA32_PAGE_WRITABLE | IA32_PAGE_PRESENT; //RPD trick
// }
// 
// void page_directory_t::init(uint32_t* placement_area)
// {
//     directory = placement_area;
//     for (int i = 0; i < 1023; i++)
//         directory[i] = 0;
//     directory[1023] = (address_t)directory | IA32_PAGE_WRITABLE | IA32_PAGE_PRESENT; //RPD trick
// }
// 
// page_t* page_directory_t::create_mapping(address_t virt, address_t phys)
// {
//     page_table_t* pt = page_table(virt, true);
//     ASSERT(pt);
// 
//     uint32_t pte = pte_entry(virt);
//     if (!pt->page(pte).present()) // page isn't mapped
//     {
//         pt->page(pte) = (phys & PAGE_MASK) | IA32_PAGE_WRITABLE | IA32_PAGE_PRESENT;
//         ia32_mmu_t::flush_page_directory_entry(virt);
//     }
//     else
//     {
//         // page is already mapped
//         return 0;
//     }
// 
// #if MEMORY_DEBUG
//     kconsole << "+(" << virt << "=>" << phys << ") to entry " << pte << " @" << (address_t)pt << "(" << (address_t)&pt->page(pte) << ")" << endl;
// #endif
// 
//     return &pt->page(pte);
// }
// 
// void page_directory_t::remove_mapping(address_t virt)
// {
//     page_table_t* pt = page_table(virt, false);
// 
//     if (pt)
//     {
//         uint32_t pte = pte_entry(virt);
// 
//         if (pt->page(pte).present())
//         {
//             // page is mapped, so unmap it
//             pt->page(pte) = IA32_PAGE_WRITABLE; // r/w, not present
//             ia32_mmu_t::flush_page_directory_entry(virt);
//         }
//     }
// }
// 
// bool page_directory_t::is_mapped(address_t virt)
// {
//     page_table_t* pt = page_table(virt, false);
//     if (!pt)
//         return false;
// 
//     uint32_t pte = pte_entry(virt);
//     if (!pt->page(pte).present())
//         return false;
// 
//     return true;
// }
// 
// page_t* page_directory_t::mapping(address_t virt, bool make)
// {
//     page_table_t* pt = page_table(virt, make);
//     if (!pt)
//         return 0;
// 
//     uint32_t pte = pte_entry(virt);
//     if (!pt->page(pte).present() && !make)
//         return 0;
// 
//     return &pt->page(pte);
// }
// 
// void page_directory_t::dump()
// {
//     kconsole << WHITE << "Dumping page directory:" << endl;
//     for (int i = 0; i < 1024; i++)
//     {
//         if(!(directory[i] & IA32_PAGE_PRESENT))
//             continue;
// 
//         page_table_t* pt = page_table(i << PDE_SHIFT, false);
//         ASSERT(pt);
// 
//         kconsole << "  Page table " << i << endl;
//         for (int k = 0; k < 1024; k++)
//         {
//             page_t& page = pt->page(k);
// 
//             if (page.present())
//                 kconsole << "    " << (unsigned int)(i << PDE_SHIFT) + (k << PTE_SHIFT) << " => " << page.frame() << endl;
//         }
// 
//         kconsole << "  End of table." << endl;
//     }
//     kconsole << "End of directory." << endl;
// }
// 
// void page_directory_t::set_page_table(address_t virt, address_t phys)
// {
//     directory[pde_entry(virt)] = (phys & PAGE_MASK) | IA32_PAGE_WRITABLE | IA32_PAGE_PRESENT;
//     ia32_mmu_t::flush_page_directory_entry(virt);
// }
// 
// page_table_t* page_directory_t::page_table(address_t virt, bool make)
// {
//     uint32_t pde = pde_entry(virt);
//     page_table_t* page_table = 0;
// 
//     if (directory_access[pde] & IA32_PAGE_PRESENT)
//     {
//         if (directory_access == directory_physical)
//             page_table = reinterpret_cast<page_table_t*>(directory[pde] & PAGE_MASK);
//         else
//             page_table = (page_table_t*)(VIRTUAL_PAGE_TABLES + (pde * PAGE_SIZE));
//     }
//     else if (make) // doesn't exist, so alloc a page and add into pdir
//     {
//         address_t phys;
//         page_table = new(&phys) page_table_t;
// #if MEMORY_DEBUG
//         kconsole << "allocating new pagetable " << pde << " @ " << phys << endl;
// #endif
// 
//         set_page_table(virt, phys);
//         page_table->zero();
//     }
// 
//     return page_table;
// }

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
