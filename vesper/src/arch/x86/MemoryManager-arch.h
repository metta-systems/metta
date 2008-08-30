#pragma once
#ifndef __INCLUDED_ARCH_X86_MEMORYMANAGER_H
#define __INCLUDED_ARCH_X86_MEMORYMANAGER_H
// # ifdef LANG_X86

#include "Globals.h"

#define PAGE_SIZE 0x1000
#define PAGE_MASK 0xFFFFF000

/**
 * A page is a pointer to a frame.
 */
class Page
{
public:
	enum pagesize_e {
		size_4k = 0,
		size_4m = 1
	};

	Page() : raw(0) {}

	// Predicates
	bool isPresent()  {return pg.present == 1;}
	bool isWritable() {return pg.rw == 1;}
	bool isUser()     {return pg.privilege == 1;}
	bool isKernel()   {return pg.privilege == 0;}
	bool isAccessed() {return pg.accessed == 1;}
	bool isDirty()    {return pg.dirty == 1;}

	// Retrieval
	Address frame()   {return pg.base << 12;}

	// Modification
	void setPresent(bool b)  { pg.present   = b ? 1 : 0; }
	void setWritable(bool b) { pg.rw        = b ? 1 : 0; }
	void setUser(bool b)     { pg.privilege = b ? 1 : 0; }
	void setAccessed(bool b) { pg.accessed  = b ? 1 : 0; }
	void setDirty(bool b)    { pg.dirty     = b ? 1 : 0; }
	void setFrame(Address f) { pg.base      = (f >> 12); }

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

extern "C" void copyPagePhysical(uint32_t from, uint32_t to);

/**
  A page table holds 1024 pages
**/
class PageTable
{
public:
	PageTable()
	{
		for(int i = 0; i < 1024; i++)
		{
			pages[i] = Page();
		}
	}

	Page *getPage(uint32_t n)
	{
		return &pages[n];
	}

	PageTable *clone(Address *phys)
	{
		PageTable *table = new(true, phys) PageTable();
		for(int i = 0; i < 1024; i++)
		{
			if (pages[i].frame())
			{
				memoryManager.allocFrame(&table->pages[i]);
				table->pages[i].setPresent(pages[i].isPresent());
				table->pages[i].setWritable(pages[i].isWritable());
				table->pages[i].setUser(pages[i].isUser());
				table->pages[i].setAccessed(pages[i].isAccessed());
				table->pages[i].setDirty(pages[i].isDirty());
				copyPagePhysical(pages[i].frame(), table->pages[i].frame());
			}
		}
		return table;
	}

	private:
		Page pages[1024];
};

/**
  Holds 1024 pagetables.
**/
class PageDirectory
{
public:
	PageDirectory()
	{
		for (int i = 0; i < 1024; i++)
		{
			tables[i] = 0;
			tablesPhysical[i] = 0;
		}
		physicalAddr = (Address)tablesPhysical;
	}

	PageTable *getTable(uint32_t index)
	{
		//ASSERT(index >= 0 && index < 1024);
		return tables[index];
	}

	Address getPhysical()
	{
		return physicalAddr;
	}

	Page *getPage(Address addr, bool make = true)
	{
		addr /= PAGE_SIZE;
		uint32_t tableIdx = addr / 1024;
		if (tables[tableIdx]) // if the table is already assigned
		{
			return tables[tableIdx]->getPage(addr % 1024);
		}
		else if(make)
		{
			Address tmp;
			tables[tableIdx] = new(true/*page aligned*/, &tmp/*give phys. addr */) PageTable();
			tablesPhysical[tableIdx] = tmp | 0x7; // PRESENT, RW, US
			return tables[tableIdx]->getPage(addr % 1024);
		}
		else
		{
			return NULL;
		}
	}

	/**
		Create a new page directory, that is an identical copy to 'dir'.
		TODO: implement copy-on-write.
	**/
	PageDirectory *clone()
	{
		Address phys;
		PageDirectory *dir = new(true, &phys) PageDirectory();

		// Get the offset of tablesPhysical from the start of
		// the PageDirectory object.
		uint32_t offset = (Address)dir->tablesPhysical - (Address)dir;

		// Then the physical address of dir->tablesPhysical is
		dir->physicalAddr = phys + offset;

		// go through each page table. If the page table is in the
		// kernel directory, do not make a new copy.
		for (int i = 0; i < 1024; i++)
		{
			if (!tables[i]) continue;
			if (memoryManager.getKernelDirectory()->getTable(i) == tables[i])
			{
				// It's in the kernel, so just use the same pointer.
				dir->tables[i] = tables[i];
				dir->tablesPhysical[i] = tablesPhysical[i];
			}
			else
			{
				// copy the table.
				Address phys;
				dir->tables[i] = tables[i]->clone(&phys);
				dir->tablesPhysical[i] = phys | 0x07;
			}
		}

		return dir;
	}

	void dump()
	{
/*		bool b = false;
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
		Array of pointers to pagetables.
	**/
	PageTable *tables[1024];

	/**
		Array of pointers to the pagetables above, but gives their *physical*
		location, for loading into the CR3 register.
	**/
	Address tablesPhysical[1024];

	/**
		The physical address of tablesPhysical.
	**/
	Address physicalAddr;
};

// # endif // LANG_X86
#endif  // __INCLUDED_ARCH_X86_MEMORYMANAGER_H
