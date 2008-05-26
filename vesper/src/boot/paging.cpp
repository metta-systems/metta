// Copied almost verbatim from JamesM tutorial.
#include "paging.h"
#include "kalloc.h"
#include "DefaultConsole.h"

/**
	Paging works by splitting the virtual address space into blocks called \c pages, which are usually 4KB in size. Pages can then be mapped on to \c frames - equally sized blocks of physical memory.
**/

// The kernel's page directory
PageDirectory *kernel_directory=0;

// The current page directory;
PageDirectory *current_directory=0;

// A bitset of frames - used or free.
uint32_t *frames;
uint32_t nframes;

// Defined in kalloc.cpp
extern uint32_t placement_address;

// Macros used in the bitset algorithms.
#define INDEX_FROM_BIT(a) ((a)/(8*4))
#define INDEX_TO_BIT(a) ((a)*8*4)
#define OFFSET_FROM_BIT(a) ((a)%(8*4))

// Static function to set a bit in the frames bitset
static void set_frame(uint32_t frame_addr)
{
	uint32_t frame = frame_addr/0x1000;
	uint32_t idx = INDEX_FROM_BIT(frame);
	uint32_t off = OFFSET_FROM_BIT(frame);
	frames[idx] |= (0x1 << off);
}

// Static function to clear a bit in the frames bitset
static void clear_frame(uint32_t frame_addr)
{
	uint32_t frame = frame_addr/0x1000;
	uint32_t idx = INDEX_FROM_BIT(frame);
	uint32_t off = OFFSET_FROM_BIT(frame);
	frames[idx] &= ~(0x1 << off);
}

// Static function to test if a bit is set.
/*static uint32_t test_frame(uint32_t frame_addr)
{
	uint32_t frame = frame_addr/0x1000;
	uint32_t idx = INDEX_FROM_BIT(frame);
	uint32_t off = OFFSET_FROM_BIT(frame);
	return (frames[idx] & (0x1 << off));
}*/

// Static function to find the first free frame.
// FIXME needs bsr implementation to work faster.
static uint32_t first_frame()
{
	uint32_t i, j;
	for (i = 0; i < INDEX_FROM_BIT(nframes); i++)
	{
		if (frames[i] != 0xFFFFFFFF) // nothing free, exit early.
		{
			// at least one bit is free here.
			for (j = 0; j < 32; j++)
			{
				uint32_t toTest = 0x1 << j;
				if ( !(frames[i]&toTest) )
				{
					return INDEX_TO_BIT(i)+j;
				}
			}
		}
	}
	return (uint32_t)-1;
}

// Function to allocate a frame of physical memory for a given page.
void alloc_frame(PageTableEntry *page, int is_kernel, int is_writeable)
{
	if (page->pg.base != 0)
	{
		return; // Page already has an associated frame.
	}
	else
	{
		uint32_t idx = first_frame();
		if (idx == (uint32_t)-1)
		{
			PANIC("No free frames!");
		}
		set_frame(idx*0x1000);
		// TODO: replace with set()ter
		page->pg.present = 1;
		page->pg.rw = (is_writeable)?1:0;
		page->pg.privilege = (is_kernel)?0:1;
		page->pg.base = idx;
	}
}

// Function to deallocate a frame.
void free_frame(PageTableEntry *page)
{
	uint32_t frame;
	if (!(frame=page->pg.base))
	{
		return;
	}
	else
	{
		clear_frame(frame);
		page->pg.base = 0x0;
	}
}

/**
   Handler for page faults.
**/
void page_fault(registers_t regs);

Paging& Paging::self()
{
	static Paging pager;
	return pager;
}

Paging::Paging()
{
	// The size of physical memory. For the moment we
	// assume it is 16MB big.
	uint32_t mem_end_page = 0x1000000;//TODO grab this from multiboot structure

	nframes = mem_end_page / 0x1000;
	frames = (uint32_t*)kmalloc(INDEX_FROM_BIT(nframes));
	memset(frames, 0, INDEX_FROM_BIT(nframes));

	// Let's make a page directory.
	kernel_directory = (PageDirectory*)kmalloc_a(sizeof(PageDirectory));
	current_directory = kernel_directory;

	// We need to identity map (phys addr = virt addr) from
	// 0x0 to the end of used memory, so we can access this
	// transparently, as if paging wasn't enabled.
	// NOTE that we use a while loop here deliberately.
	// inside the loop body we actually change placement_address
	// by calling kmalloc(). A while loop causes this to be
	// computed on-the-fly rather than once at the start.
	uint32_t i = 0;
	while (i < placement_address)
	{
		// Kernel code is readable but not writeable from userspace.
		alloc_frame( get_page(i, 1, kernel_directory), 0, 0);
		i += 0x1000;
	}

	// Before we enable paging, we must register our page fault handler.
	register_interrupt_handler(14, page_fault);

	// Now, enable paging!
	switch_page_directory(kernel_directory);
}

void Paging::switch_page_directory(PageDirectory *dir)
{
	current_directory = dir;
	asm volatile("mov %0, %%cr3":: "r"(&dir->tablesPhysical));
	uint32_t cr0;
	asm volatile("mov %%cr0, %0": "=r"(cr0));
	cr0 |= 0x80000000; // Enable paging!
	asm volatile("mov %0, %%cr0":: "r"(cr0));
}

PageTableEntry *Paging::get_page(uint32_t address, int make, PageDirectory *dir)
{
	// Turn the address into an index.
	address /= 0x1000;
	// Find the page table containing this address.
	uint32_t table_idx = address / 1024;
	if (dir->tables[table_idx]) // If this table is already assigned
	{
		return &dir->tables[table_idx]->pages[address%1024];
	}
	else if(make)
	{
		uint32_t tmp;
		dir->tables[table_idx] = (PageTable*)kmalloc_ap(sizeof(PageTable), &tmp);
		dir->tablesPhysical[table_idx] = tmp | 0x7; // PRESENT, RW, US.
		return &dir->tables[table_idx]->pages[address%1024];
	}
	else
	{
		return 0;
	}
}

void page_fault(registers_t regs)
{
	// A page fault has occurred.
	// The faulting address is stored in the CR2 register.
	uint32_t faulting_address;
	asm volatile("mov %%cr2, %0" : "=r" (faulting_address));

	// The error code gives us details of what happened.
	int present   = !(regs.err_code & 0x1); // Page not present
	int rw = regs.err_code & 0x2;           // Write operation?
	int us = regs.err_code & 0x4;           // Processor was in user-mode?
	int reserved = regs.err_code & 0x8;     // Overwritten CPU-reserved bits of page entry?
	int id = regs.err_code & 0x10;          // Caused by an instruction fetch?

	// Output an error message.
	kconsole.set_attr(LIGHTCYAN, BLACK);
	kconsole.print("Page fault! ( ");
	kconsole.set_attr(WHITE, BLACK);
	if (present) kconsole.print("present ");
	if (rw) kconsole.print("read-only ");
	if (us) kconsole.print("user-mode ");
	if (reserved) kconsole.print("reserved ");
	if (id) kconsole.print("insn ");
	kconsole.set_attr(LIGHTCYAN, BLACK);
	kconsole.print(") at 0x");
	kconsole.print_hex(faulting_address);
	kconsole.newline();
	PANIC("Page fault");
}
