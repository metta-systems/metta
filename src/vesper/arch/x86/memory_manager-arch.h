//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once
// # ifdef LANG_X86

#include "globals.h" // for new/delete

namespace metta {
namespace kernel {

// for arch-dependent stuff..
// namespace md {

#define PAGE_SIZE 0x1000
#define PAGE_MASK 0xFFFFF000

/**
* A page is a pointer to a frame.
**/
class page
{
public:
    enum pagesize_e {
        size_4k = 0,
        size_4m = 1
    };

    page() : raw(0) {}

    // Predicates
    bool isPresent()  {return pg.present == 1;}
    bool isWritable() {return pg.rw == 1;}
    bool isUser()     {return pg.privilege == 1;}
    bool isKernel()   {return pg.privilege == 0;}
    bool isAccessed() {return pg.accessed == 1;}
    bool isDirty()    {return pg.dirty == 1;}

    // Retrieval
    address_t frame()   {return pg.base << 12;}

    // Modification
    void setPresent(bool b)  { pg.present   = b ? 1 : 0; }
    void setWritable(bool b) { pg.rw        = b ? 1 : 0; }
    void setUser(bool b)     { pg.privilege = b ? 1 : 0; }
    void setAccessed(bool b) { pg.accessed  = b ? 1 : 0; }
    void setDirty(bool b)    { pg.dirty     = b ? 1 : 0; }
    void setFrame(address_t f) { pg.base      = (f >> 12); }

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

extern "C" void copy_page_physical(uint32_t from, uint32_t to);

/**
* A page table holds 1024 pages
**/
class page_table
{
public:
    page_table()
    {
        for(int i = 0; i < 1024; i++)
        {
            pages[i] = page();
        }
    }

    page *get_page(uint32_t n)
    {
        return &pages[n];
    }

    page_table *clone(address_t *phys)
    {
        page_table *table = new(true, phys) page_table();
        for(int i = 0; i < 1024; i++)
        {
            if (pages[i].frame())
            {
                kmemmgr.alloc_frame(&table->pages[i]);
                table->pages[i].setPresent(pages[i].isPresent());
                table->pages[i].setWritable(pages[i].isWritable());
                table->pages[i].setUser(pages[i].isUser());
                table->pages[i].setAccessed(pages[i].isAccessed());
                table->pages[i].setDirty(pages[i].isDirty());
                copy_page_physical(pages[i].frame(), table->pages[i].frame());
            }
        }
        return table;
    }

    private:
        page pages[1024];
};

/**
* Holds 1024 pagetables.
**/
class page_directory
{
public:
    page_directory()
    {
        for (int i = 0; i < 1024; i++)
        {
            tables[i] = 0;
            tables_physical[i] = 0;
        }
        physical_addr = (address_t)tables_physical;
    }

    page_table *get_table(uint32_t index)
    {
        //ASSERT(index >= 0 && index < 1024);
        return tables[index];
    }

    address_t get_physical()
    {
        return physical_addr;
    }

    page *get_page(address_t addr, bool make = true)
    {
        addr /= PAGE_SIZE;
        uint32_t table_idx = addr / 1024;
        if (tables[table_idx]) // if the table is already assigned
        {
            return tables[table_idx]->get_page(addr % 1024);
        }
        else if(make)
        {
            address_t tmp;
            tables[table_idx] = new(true/*page aligned*/, &tmp/*give phys. addr */) page_table();
            tables_physical[table_idx] = tmp | 0x7; // PRESENT, RW, US
            return tables[table_idx]->get_page(addr % 1024);
        }
        else
        {
            return NULL;
        }
    }

    /**
    * Create a new page directory, that is an identical copy of this.
    * TODO: implement copy-on-write.
    **/
    page_directory *clone()
    {
        address_t phys;
        page_directory *dir = new(true, &phys) page_directory();

        // Get the offset of tables_physical from the start of
        // the page_directory object.
        uint32_t offset = (address_t)dir->tables_physical - (address_t)dir;

        // Then the physical address of dir->tables_physical is
        dir->physical_addr = phys + offset;

        // Go through each page table. If the page table is in the
        // kernel directory, do not make a new copy.
        for (int i = 0; i < 1024; i++)
        {
            if (!tables[i]) continue;
            if (kmemmgr.get_kernel_directory()->get_table(i) == tables[i])
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

    void dump()
    {
/*      bool b = false;
        kconsole.print("Dumping page directory:\n");
        for (int i = 0; i < 0xFFFFF; i++)
        {
            Page* page;
            if (tables[i / 1024])
                page = tables[i / 1024]->getPage(i % 1024);
            else if (b)
            {
                kconsole.print("    End of table.\n");
                b = false;
            }
            else
                continue;

            if (!b && page->isPresent())
            {
                b = true;
                kerr.write("    %08x\n        .\n", i << 12);
            }
            else if(b && !page->getPresent())
            {
                b = false;
                kerr.write("\t");
                kerr.writeHex(i<<12);
                kerr.write("\n");
            }
        }*/
    }

private:
    /**
    * Array of pointers to pagetables.
    **/
    page_table *tables[1024];

    /**
    * Array of pointers to the pagetables above, but gives their *physical*
    * location, for loading into the CR3 register.
    **/
    address_t tables_physical[1024];

    /**
    * The physical address of tablesPhysical.
    **/
    address_t physical_addr;
};

template <typename T>
inline T page_align_up(T a)
{
    if (a % PAGE_SIZE)
    {
        a &= PAGE_MASK;
        a += PAGE_SIZE;
    }
    return a;
}

template <typename T>
inline T page_align_down(T a)
{
    return a - a % PAGE_SIZE;
}

template <typename T>
inline bool page_aligned_p(T a)
{
    return a % PAGE_SIZE == 0;
}

}
}

// # endif // LANG_X86

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
