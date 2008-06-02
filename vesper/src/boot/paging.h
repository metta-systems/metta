#ifndef __INCLUDED_PAGING_H
#define __INCLUDED_PAGING_H

#include "common.h"
#include "isr.h"
#include "ia32.h"

class PageTableEntry
{
public:
	enum pagesize_e {
		size_4k = 0,
		size_4m = 1
	};

	// Predicates
	bool is_valid()     { return pg.present == 1; }
	bool is_writable()  { return pg.rw == 1; }
	bool is_accessed()  { return pg.accessed == 1; }
	bool is_dirty()     { return pg.dirty == 1; }
	bool is_superpage() { return pg.size == 1; }
	bool is_kernel()    { return pg.privilege == 0; }

	// Retrieval
    uint32_t get_raw()     { return raw; }

	// Modification
	void clear()           { raw = 0; }

	void set_entry(addr_t addr, pagesize_e size, uint32_t attrib)
	{
	    if (size == size_4k)
			raw = ((uint32_t)(addr) & IA32_PAGE_MASK) | (attrib & IA32_PAGE_FLAGS_MASK);
	    else
			raw = ((uint32_t)(addr) & IA32_SUPERPAGE_MASK) | IA32_PAGE_SUPER |
		    (attrib & IA32_SPAGE_FLAGS_MASK);
	}

	void set_ptab_entry(addr_t addr, uint32_t attrib)
	{
	    raw = ((uint32_t)(addr) & IA32_PAGE_MASK) | IA32_PAGE_VALID | (attrib & IA32_PTAB_FLAGS_MASK);
	}

	void copy(const PageTableEntry pte)
	{
	    raw = pte.raw;
	}

	uint32_t get_pdir_idx(addr_t addr)
	{
	    return (uint32_t)addr >> IA32_PAGEDIR_BITS;
	}

	uint32_t get_ptab_idx(addr_t addr)
	{
	    return (((uint32_t)addr) & (~IA32_PAGEDIR_MASK)) >> IA32_PAGE_BITS;
	}

	void set_cacheability(bool cacheable, pagesize_e size)
	{
	    this->pg.cache_disabled = !cacheable;
#if defined(CONFIG_X86_PAT)
	    if (size == size_4k)
			pg.size = 0;
	    else
			pg4m.pat = 0;
#else
		UNUSED(size);
#endif
	}

	void set_pat (uint16_t pat, pagesize_e size)
	{
	    pg.write_through  = (pat & 1) ? 1 : 0;
	    pg.cache_disabled = (pat & 2) ? 1 : 0;
#if defined(CONFIG_X86_PAT)
	    if (size == size_4k)
			pg.size  = (pat & 4) ? 1 : 0;
	    else
			pg4m.pat = (pat & 4) ? 1 : 0;
#else
		UNUSED(size);
#endif
	}

	void set_global (bool global)
	{
	    this->pg.global = global;
	}

public: //private:
	union {
		struct {
			uint32_t present		:1;//(present)
			uint32_t rw			:1;//(rw)
			uint32_t privilege		:1;//(user) supervisor only when 0
			uint32_t write_through	:1;//(accessed)

			uint32_t cache_disabled	:1;
			uint32_t accessed		:1;
			uint32_t dirty		:1;//(dirty)
			uint32_t size		:1;

			uint32_t global		:1;
			uint32_t avail		:3;

			uint32_t base		:20; // frame
		} pg;

		struct {
			uint32_t present		:1;
			uint32_t rw			:1;
			uint32_t privilege		:1;
			uint32_t write_through	:1;

			uint32_t cache_disabled	:1;
			uint32_t accessed		:1;
			uint32_t dirty		:1;
			uint32_t size		:1;

			uint32_t global		:1;
			uint32_t avail		:3;

			uint32_t pat		:1;
			uint32_t reserved		:9;
			uint32_t base		:10;
		} pg4m;

		uint32_t raw;
	};
// typedef struct page
// {
//     u32int present    : 1;   // Page present in memory
//     u32int rw         : 1;   // Read-only if clear, readwrite if set
//     u32int user       : 1;   // Supervisor level only if clear
//     u32int accessed   : 1;   // Has the page been accessed since last refresh?
//     u32int dirty      : 1;   // Has the page been written to since last refresh?
//     u32int unused     : 7;   // Amalgamation of unused and reserved bits
//     u32int frame      : 20;  // Frame address (shifted right 12 bits)
// } page_t;
};

class PageTable
{
	public: //private:
		PageTableEntry pages[1024];
};

class PageDirectory
{
	public: //private:
		/**
			Array of pointers to pagetables.
		**/
		PageTable *tables[1024];
		/**
			Array of pointers to the pagetables above, but gives their *physical*
			location, for loading into the CR3 register.
		**/
		uint32_t tablesPhysical[1024];
		/**
			The physical address of tablesPhysical. This comes into play
			when we get our kernel heap allocated and the directory
			may be in a different location in virtual memory.
		**/
		uint32_t physicalAddr;
};

#define paging Paging::self()

class Paging
{
	public:
		static Paging &self();

		/**
			Causes the specified page directory to be loaded into the
			CR3 register.
		**/
		void switch_page_directory(PageDirectory *dir);

		/**
			Retrieves a pointer to the page required.
			If make == 1, if the page-table in which this page should
			reside isn't created, create it!
		**/
		PageTableEntry *get_page(uint32_t address, int make, PageDirectory *dir);

	private:
		/**
			Sets up the environment, page directories etc and
			enables paging.
		**/
		Paging();
};

// externs for kalloc.cpp
extern PageDirectory *kernel_directory;
extern void alloc_frame(PageTableEntry *page, int is_kernel, int is_writeable);
extern void free_frame(PageTableEntry *page);


#endif
