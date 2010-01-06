//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "types.h"
#include "ia32.h"

#define PDE_SHIFT 22
#define PDE_MASK  0x3ff
#define PTE_SHIFT 12
#define PTE_MASK  0x3ff

inline int pde_entry(address_t virt)
{
    return (virt >> PDE_SHIFT) & PDE_MASK;
}

inline int pte_entry(address_t virt)
{
    return (virt >> PTE_SHIFT) & PTE_MASK;
}

/*!
 * A page is a pointer to a frame.
 *
 * Page constructor does not initialize page_t to any predefined value.
 * This is done first to be able to create page_t over any existing page tables
 * via placement new and, second to prevent compiler from generating initialization
 * for page_table_t, which would break paging.
 */
class page_t
{
public:
    page_t() {}

    // Predicates
    bool present()    { return pg.present == 1; }
    bool writable()   { return pg.writable == 1; }
    bool user()       { return pg.user == 1; }
    bool kernel()     { return pg.user == 0; }
    bool accessed()   { return pg.accessed == 1; }
    bool dirty()      { return pg.dirty == 1; }

    // Retrieval
    address_t frame() { return pg.base << PTE_SHIFT; }

    // Modification
    void set_present(bool b)    { pg.present   = b ? 1 : 0; }
    void set_writable(bool b)   { pg.writable  = b ? 1 : 0; }
    void set_user(bool b)       { pg.user      = b ? 1 : 0; }
    void set_accessed(bool b)   { pg.accessed  = b ? 1 : 0; }
    void set_dirty(bool b)      { pg.dirty     = b ? 1 : 0; }
    void set_frame(address_t f) { pg.base      = f >> PTE_SHIFT; }

    page_t& operator =(uint32_t v)  { raw = v; return *this; }
    operator uint32_t()             { return raw; }

private:
    union {
        struct {
            uint32_t present        :1;
            uint32_t writable       :1;
            uint32_t user           :1;
            uint32_t write_through  :1;
            uint32_t cache_disabled :1;
            uint32_t accessed       :1;//in PDE:
            uint32_t dirty          :1;//reserved
            uint32_t pat            :1;//pagesize
            uint32_t global         :1;
            uint32_t avail          :3;
            uint32_t base           :20;
        } pg;
        uint32_t raw;
    };
};

/*!
 * A page table holds 1024 pages
 *
 * Page table constructor does not do any initialization to allow creating
 * page_table_t on top of already existing page tables.
 */
class page_table_t
{
public:
    page_table_t() {}

    /*!
     * Zero out page table, setting all page frames to not present.
     */
    void zero()
    {
        for(int i = 0; i < 1024; i++)
            pages[i] = IA32_PAGE_WRITABLE;
    }

    page_t& page(uint32_t n)
    {
        return pages[n];
    }

    page_t& operator[](uint32_t n)
    {
        return pages[n];
    }

    /*!
     * Allocate new page table from frame allocator.
     */
    void* operator new(size_t size, address_t* physical_address);

private:
    page_t pages[1024];
};

/*!
 * Page directory holds 1024 pagetables.
 *
 * Page directory performs initialization in init(), mainly to set up
 * recursive PD structure.
 *
 * Kernel temporary service mappings are located just below the RPD mark (4 reserved pages).
 */
class page_directory_t
{
public:
    page_directory_t() : directory_physical(0), directory_virtual(0) {}
    page_directory_t(physical_address_t* existing_directory) : directory_physical(existing_directory) {}
    virtual ~page_directory_t() {}

    void init();
    void init(physical_address_t* placement_area);

    /*!
     * Returns physical address of PD, for setting PDBR.
     */
    physical_address_t get_physical()
    {
        return directory[1023] & PAGE_MASK;
    }

    void enable_paging();
    void disable_paging();

    /*!
     * Create mapping from virtual address @p virt to physical frame at @p phys
     * with given flags.
     *
     * @returns NULL on failure or when mapping from @p virt already exists, otherwise
     *   page_t* which allows setting flags from client side.
     */
    page_t* create_mapping(address_t virt, address_t phys);

    /*!
     * Remove mapping of virtual address @p virt from page directory & tables.
     */
    void remove_mapping(address_t virt);

    /*!
     * @returns true if mapping of virtual address @p virt exists, false otherwise.
     */
    bool is_mapped(address_t virt);

    /*!
     * Obtain mapping information from virtual address @p virt. @p make specifies if
     * page table should be created if it doesn't exist yet.
     * @returns page table entry if found or created, NULL otherwise.
     */
    page_t* mapping(address_t virt, bool make = false);

    /*!
     * Print a debug dump of page directory.
     */
    void dump();

protected:
    /*!
     * Obtain page table corresponding to address @p virt. If @p make is true, create
     * page table if it doesn't exist.
     */
    page_table_t* page_table(address_t virt, bool make);

    /*!
     * Pointer to a frame with array of pointers to pagetables, gives their @e physical location,
     * for loading into the CR3 register.
     */
    physical_address_t* directory_physical;
    address_t* directory_virtual;
    address_t* directory_access;

private: friend class stack_frame_allocator_t;//FIXME: page_allocator_t;
    /*!
     * Set @p phys to be address of the frame for page table for address @p virt.
     */
    void set_page_table(address_t virt, address_t phys);
};

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
