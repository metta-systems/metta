//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2010, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "x86_protection_domain.h"
#include "x86_frame_allocator.h"
#include "page_directory.h"
#include "memutils.h"
#include "mmu.h"
#include "default_console.h"

protection_domain_t& protection_domain_t::privileged()
{
    static x86_protection_domain_t privileged_domain(0);
    return privileged_domain;
}

protection_domain_t* protection_domain_t::create()
{
    return new x86_protection_domain_t;
}

x86_protection_domain_t::x86_protection_domain_t()
{
    virtual_page_directory = reinterpret_cast<page_t*>(VIRTUAL_PAGE_DIRECTORY);
    virtual_page_tables = reinterpret_cast<page_t*>(VIRTUAL_PAGE_TABLES);
    // TODO: finish
}

//! This is called before enabling paging.
x86_protection_domain_t::x86_protection_domain_t(int /*privileged*/)
{
    // we can actually allocate memory frames in this constructor, so create and initialise pagedir and startup pagetables freely
    x86_frame_allocator_t& allocator = x86_frame_allocator_t::instance();

    physical_page_directory = allocator.allocate_frame();
    kconsole << " got physical_page_directory from frame allocator: " << physical_page_directory << endl;
    memutils::fill_memory(reinterpret_cast<void*>(physical_page_directory), 0, allocator.page_size());

    virtual_page_directory = reinterpret_cast<page_t*>(physical_page_directory);
    virtual_page_tables = reinterpret_cast<page_t*>(allocator.allocate_frame());
    kconsole << " got page_tables from frame allocator: " << virtual_page_tables << endl;
    memutils::fill_memory(reinterpret_cast<void*>(virtual_page_tables), 0, allocator.page_size());
    // we only use page table 0 before enabling paging so this trick works.

    page_t pde;
    pde.set_frame(virtual_page_tables);
    pde.set_flags(page_t::kernel_mode | page_t::writable);
    virtual_page_directory[0] = pde;

    // clear escrow pages for mapping
}

void x86_protection_domain_t::dump()
{
    kconsole << "== Dumping protection domain ==" << endl;
    kconsole << " ** page directory:" << endl;
    for (int i = 0; i < 1024; i++)
    {
        if (!virtual_page_directory[i].is_present())
            continue;

        page_t* pt = &virtual_page_tables[i * 1024];
        ASSERT(pt);

        kconsole << "  * page table " << i << " ";
        virtual_page_directory[i].dump_flags();
        kconsole << endl;
        for (int k = 0; k < 1024; k++)
        {
            page_t& page = pt[k];

            if (page.is_present())
            {
                kconsole << "    " << (unsigned int)(i << PDE_SHIFT) + (k << PTE_SHIFT) << " => " << page.frame() << " flags: ";
                page.dump_flags();
                kconsole << endl;
            }
        }

        kconsole << "  * end of table " << i << endl;
    }
    kconsole << " ** end of page directory." << endl;
    kconsole << "== End of protection domain dump ==" << endl;
}

void x86_protection_domain_t::enable_paging()
{
    // Set up RPD.
    page_t rpd;
    rpd.set_frame(physical_page_directory);
    rpd.set_flags(page_t::kernel_mode | page_t::writable);
    virtual_page_directory[1023] = rpd;

    virtual_page_directory = reinterpret_cast<page_t*>(VIRTUAL_PAGE_DIRECTORY);
    virtual_page_tables = reinterpret_cast<page_t*>(VIRTUAL_PAGE_TABLES);
    ia32_mmu_t::set_active_pagetable(*this);
//     ia32_mmu_t::enable_4mb_pages();
//     ia32_mmu_t::enable_global_pages();
    ia32_mmu_t::enable_paged_mode();
}

bool x86_protection_domain_t::is_valid(void* /*virtual_address*/)
{
    return true;
}

bool x86_protection_domain_t::is_mapped(void* virtual_address)
{
    page_t pde = virtual_page_directory[pde_entry(virtual_address)];
    if (!pde.is_present())
        return false;
    if (pde.is_4mb())
        return true;
    return virtual_page_tables[pde_entry(virtual_address) * PTE_ENTRIES + pte_entry(virtual_address)].is_present();
}

bool x86_protection_domain_t::map(physical_address_t physical_address, void* virtual_address, flags_t flags)
{
    if (!virtual_page_directory[pde_entry(virtual_address)].is_present())
    {
        // Need to allocate a pagetable.
        page_t pde;
        physical_address_t pta = x86_frame_allocator_t::instance().allocate_frame();
        pde.set_frame(pta);
        pde.set_flags(page_t::kernel_mode | page_t::writable);
        virtual_page_directory[pde_entry(virtual_address)] = pde;
    }

    size_t pti = pde_entry(virtual_address) * PTE_ENTRIES + pte_entry(virtual_address);

    if (virtual_page_tables[pti].is_present())
        return false; // already mapped!

    page_t pg;
    pg.set_frame(physical_address);
    pg.set_flags(flags);
    virtual_page_tables[pti] = pg;

    return true;
}

void x86_protection_domain_t::mapping(void* virtual_address, physical_address_t& physical_address, flags_t& flags)
{
    if (!virtual_page_directory[pde_entry(virtual_address)].is_present())
        return;
    size_t pti = pde_entry(virtual_address) * PTE_ENTRIES + pte_entry(virtual_address);
    if (!virtual_page_tables[pti].is_present())
        return;
    physical_address = virtual_page_tables[pti].frame();
    flags = virtual_page_tables[pti].flags();
}

void x86_protection_domain_t::unmap(void* virtual_address)
{
    if (!virtual_page_directory[pde_entry(virtual_address)].is_present())
        return;
    size_t pti = pde_entry(virtual_address) * PTE_ENTRIES + pte_entry(virtual_address);
    if (!virtual_page_tables[pti].is_present())
        return;
    ia32_mmu_t::flush_page_directory_entry(virtual_address);
    virtual_page_tables[pti] = 0;
}
