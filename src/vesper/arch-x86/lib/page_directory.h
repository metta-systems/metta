//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "types.h"
#include "ia32.h"

/**
* A page is a pointer to a frame.
**/
class page_t
{
public:
    enum pagesize_e {
        size_4k = 0,
        size_4m = 1
    };

    page_t() : raw(0) {}

    // Predicates
    bool present()    { return pg.present == 1; }
    bool writable()   { return pg.rw == 1; }
    bool user()       { return pg.privilege == 1; }
    bool kernel()     { return pg.privilege == 0; }
    bool accessed()   { return pg.accessed == 1; }
    bool dirty()      { return pg.dirty == 1; }

    // Retrieval
    address_t frame() { return pg.base << 12; }

    // Modification
    void set_present(bool b)    { pg.present   = b ? 1 : 0; }
    void set_writable(bool b)   { pg.rw        = b ? 1 : 0; }
    void set_user(bool b)       { pg.privilege = b ? 1 : 0; }
    void set_accessed(bool b)   { pg.accessed  = b ? 1 : 0; }
    void set_dirty(bool b)      { pg.dirty     = b ? 1 : 0; }
    void set_frame(address_t f) { pg.base      = (f >> 12); }

    uint32_t/*FIXME rettype*/ operator =(uint32_t v) { raw = v; return v; }
    operator uint32_t()                              { return raw; }

private:
    union {
        struct {
            uint32_t present        :1;//(present)
            uint32_t rw             :1;//(rw)
            uint32_t privilege      :1;//(user) supervisor only when 0
            uint32_t write_through  :1;//(accessed)

            uint32_t cache_disabled :1;
            uint32_t accessed       :1;
            uint32_t dirty          :1;//(dirty)
            uint32_t size           :1;

            uint32_t global         :1;
            uint32_t avail          :3;

            uint32_t base           :20; // frame
        } pg;

        struct {
            uint32_t present        :1;
            uint32_t rw             :1;
            uint32_t privilege      :1;
            uint32_t write_through  :1;

            uint32_t cache_disabled :1;
            uint32_t accessed       :1;
            uint32_t dirty          :1;
            uint32_t size           :1;

            uint32_t global         :1;
            uint32_t avail          :3;

            uint32_t pat            :1;
            uint32_t reserved       :9;
            uint32_t base           :10;
        } pg4m;

        uint32_t raw;
    };
};

/*!
* A page table holds 1024 pages
*/
class page_table_t
{
public:
    page_table_t()
    {
        for(int i = 0; i < 1024; i++)
        {
            pages[i] = page_t();
        }
    }

    page_t* get_page(uint32_t n)
    {
        return &pages[n];
    }

    page_table_t* clone(address_t* phys);

    page_t& operator[](uint32_t n)
    {
        return pages[n];
    }

    void* operator new(size_t size);
    void* operator new(size_t size, bool page_align, address_t* physical_address);//FIXME: always page-aligned

private:
    void copy_frame(uint32_t from_phys, uint32_t to_phys);
    page_t pages[1024];
};

// Last 4 megs of address space are for recursive page directory
#define RPAGETAB_VBASE 0xffc00000 // page tables addressable from this base
#define RPAGEDIR_VBASE 0xfffff000 // page directory addressable from this base

/*!
* Holds 1024 pagetables.
*/
class page_directory_t
{
public:
    page_directory_t();

    page_table_t* get_table(uint32_t index)
    {
        //ASSERT(index >= 0 && index < 1024);
        return tables[index];
    }

    page_table_t* get_page_table(address_t vaddr, bool make = true);

    address_t get_physical()
    {
        return table[1023];
    }

    page_t* get_page(address_t addr, bool make = true);

    void enter_mapping(address_t vaddr, address_t paddr, int flags = IA32_PAGE_WRITABLE);
    bool mapping_exists(address_t vaddr);

    /*!
    * Create a new page directory, that is an identical copy of this.
    * TODO: implement copy-on-write.
    */
    page_directory_t* clone();
    void dump();

private: friend class memory_manager_t;
    page_directory_t(const page_directory_t& other);
    void set_physical(address_t phys) { tables[1023] = phys; }

private:
    /*!
     * Array of pointers to pagetables, but gives their *physical* location,
     * for loading into the CR3 register.
     */
    page_table_t* tables[1024];
};

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

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
