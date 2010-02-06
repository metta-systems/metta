//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2010, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "lockable.h"
#include "protection_domain.h"
#include "range_list.h"
#include "macros.h"

class page_t;

class x86_protection_domain_t : public protection_domain_t, public lockable_t
{
public:
    /* Declare destructor virtual */
    inline virtual ~x86_protection_domain_t() {}

    /*!
     * @returns true if address is valid in current address space, that is, address is within the bounds of
     * available stretches. Addresses outside of available stretches are considered invalid and cannot be
     * used for mapping.
     */
    virtual bool is_valid(void* virtual_address);
    /*!
     * @returns true if virtual_address is mapped into one of current protection domain's stretches.
     */
    virtual bool is_mapped(void* virtual_address);
    virtual bool map(physical_address_t physical_address,
                     void* virtual_address,
                     flags_t flags);
    virtual void mapping(void* virtual_address,
                         physical_address_t& physical_address,
                         flags_t& flags);
    virtual void unmap(void* virtual_address);

    void enable_paging();
    physical_address_t get_physical() { return physical_page_directory; }
    void dump();

protected: friend class protection_domain_t;
    /*!
     * @note This constructor is only used to construct the kernel protection domain.
     */
    x86_protection_domain_t(int privileged) INIT_ONLY;

private:
    /*! The default constructor. */
    x86_protection_domain_t();
    /*!
     * Disable the copy constructor.
     * @note NOT implemented
     */
    x86_protection_domain_t(const x86_protection_domain_t&);
    /*!
     * Disable assignment operator.
     * @note Not implemented
     */
    x86_protection_domain_t& operator =(const x86_protection_domain_t&);

private: friend class ia32_mmu_t;
    /** Physical address of the page directory */
    physical_address_t physical_page_directory;
    /** Virtual address of the page directory */
    page_t* virtual_page_directory;
    /** Virtual address of the page tables */
    page_t* virtual_page_tables;

    static range_list_t<address_t> allocated_virtual_addresses; // A SAS list of allocated addresses.
    static physical_address_t escrow_pages[1]; //TODO: use a pointer and allocate dynamically to nr_cpus
};
