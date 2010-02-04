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

x86_protection_domain_t x86_protection_domain_t::privileged(0);

protection_domain_t& protection_domain_t::privileged()
{
    return x86_protection_domain_t::privileged;
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
    memutils::fill_memory(reinterpret_cast<void*>(physical_page_directory), 0, allocator.page_size());

    virtual_page_directory = reinterpret_cast<page_t*>(physical_page_directory);
    virtual_page_tables = reinterpret_cast<page_t*>(allocator.allocate_frame());
    memutils::fill_memory(reinterpret_cast<void*>(virtual_page_tables), 0, allocator.page_size());
    // we only use page table 0 before enabling paging so this trick works.

    page_t pde;
    pde.set_frame(reinterpret_cast<physical_address_t>(virtual_page_tables));
    pde.set_present(true);
    virtual_page_directory[0] = pde;

    // clear escrow pages for mapping
}

void x86_protection_domain_t::enable_paging()
{
    // Set up RPD.
    page_t rpd;
    rpd.set_frame(physical_page_directory);
    rpd.set_writable(true);
    rpd.set_present(true);
    virtual_page_directory[1023] = rpd;

    virtual_page_directory = reinterpret_cast<page_t*>(VIRTUAL_PAGE_DIRECTORY);
    virtual_page_tables = reinterpret_cast<page_t*>(VIRTUAL_PAGE_TABLES);
    ia32_mmu_t::set_active_pagetable(*this);
    ia32_mmu_t::enable_paged_mode();
}

bool x86_protection_domain_t::is_valid(void* /*virtual_address*/)
{
    return false;
}

bool x86_protection_domain_t::is_mapped(void* virtual_address)
{
    page_t pde = virtual_page_directory[pde_entry(virtual_address)];
    if (!pde.present())
        return false;
    if (pde.four_mb())
        return true;
    return virtual_page_tables[pte_entry(virtual_address)].present();
}

bool x86_protection_domain_t::map(physical_address_t /*physical_address*/, void* virtual_address, flags_t /*flags*/)
{
    if (!virtual_page_directory[pde_entry(virtual_address)].present())
    {
        // Need to allocate a pagetable.
        page_t pde;
//         new(0, &phys) page_table_t;
//         pde.set_frame(phys);
        pde.set_present(true);
        virtual_page_directory[pde_entry(virtual_address)] = pde;
    }
    else
    {
/*        page_t pte = virtual_page_directory[pde_entry(virtual_address)];
        pte[pte_entry(virtual_address)]->set_physical(physical_address);
        ptr[pte_entry(virtual_address)]->set_flags(flags);*/
    }
    return false;
}

void x86_protection_domain_t::mapping(void* /*virtual_address*/, physical_address_t& /*physical_address*/, flags_t& /*flags*/)
{
}

void x86_protection_domain_t::unmap(void* /*virtual_address*/)
{
}
