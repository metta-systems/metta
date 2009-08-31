//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "memory.h"

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

private:
    void copy_frame(uint32_t from_phys, uint32_t to_phys);
    page_t pages[1024];
};

/**
* Holds 1024 pagetables.
**/
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
        return physical_address;
    }

    page_t* get_page(address_t addr, bool make = true);

    void enter_mapping(address_t vaddr, address_t paddr, int flags = IA32_PAGE_WRITABLE);

    /*!
    * Create a new page directory, that is an identical copy of this.
    * TODO: implement copy-on-write.
    **/
    page_directory_t* clone();
    void dump();

private:
    /*!
    * Array of pointers to pagetables.
    */
    page_table_t* tables[1024];

    /*!
    * Array of pointers to the pagetables above, but gives their *physical* location, for loading into the CR3 register.
    */
    address_t tables_physical[1024];

    /*!
    * The physical address of tables_physical.
    */
    address_t physical_address;
};

// Recursive PDE/PTE explained by Brendan @ http://forum.osdev.org/viewtopic.php?f=15&t=19387
//
// The area from 0xFFFFF000 to 0xFFFFFFFF becomes a mapping of all page directory entries, and the area from 0xFFC00000 to 0xFFFFFFFF becomes a mapping of all page table entries (if the corresponding page table is present). These areas overlap because the page directory is being used as a page table - the area from 0xFFFFF000 to 0xFFFFFFFF contains the page directory entries and the page table entries for the last page table (which is the same thing).
//
// To access the page directory entry for any virtual address you'd be able to do "mov eax,[0xFFFFF000 + (virtualAddress >> 20) * 4]", and (if the page table is present) to access the page table entry for any virtual address you'd be able to do "mov eax,[0xFFC00000 + (virtualAddress >> 12) * 4]". Most C programmers would use these areas as arrays and let the compiler do some of the work, so they can access any page directory entry as "PDE = pageDirectoryMapping[virtualAddress >> 20]" and any page table entry as "PTE = pageTableMapping[virtualAddress >> 12]".
//
// Now imagine that someone wants to allocate a new page at 0x12345000. You need to check if a page table exists for this area (and if the page table doesn't exist, allocate a new page of RAM to use as a page table, create a page directory entry for it, and INVLPG the mapping for the new page table), then check if the page already exists (and if the page doesn't exist, allocate a new page of RAM, create a page table entry for the new page, and INVLPG the new page).
//
// The same sort of lookups are needed for lots of things - freeing a page, checking accessed/dirty bits, changing page attributes (read, write, execute, etc), page fault handling, etc. Basically, having fast access to page directory entries and page table entries is important, because it speeds up everything your virtual memory management code does.
//
// Without making a page directory entry refer to the page directory, you need to find some other way to access page directory entries and page table entries, and there is no better way (all the other ways are slower, more complicated and waste more RAM).

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
