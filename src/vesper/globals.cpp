//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "globals.h"
#include "registers.h"
#include "default_console.h"
#include "kernel.h"
#include "multiboot.h"
#include "elf_parser.h"
#include "memory_manager.h"
#include "memutils.h"
#include "interrupt_descriptor_table.h"
#include "global_descriptor_table.h"

namespace metta {
namespace kernel {

/* Global objects FIXME: use singletons instead? */
class kernel kernel;
elf_parser kernel_elf_parser;
interrupt_descriptor_table interrupts_table;//good candidate  for singleton...

}
}

using namespace metta::kernel;
using metta::common::memutils;

// Declare the page directory and a page table, both 4kb-aligned
unsigned long kernelpagedir[1024] __attribute__ ((aligned (4096)));
unsigned long lowpagetable[1024] __attribute__ ((aligned (4096)));

void init_paging()
{
    // Pointers to the page directory and the page table
    void *kernelpagedirPtr = 0;
    void *lowpagetablePtr = 0;
    int k = 0;

    kernelpagedirPtr = (char *)kernelpagedir + 0x40000000;  // Translate the page directory from
    // virtual address to physical address
    lowpagetablePtr = (char *)lowpagetable + 0x40000000;    // Same for the page table

    for (k = 0; k < 1024; k++)
    {
        lowpagetable[k] = (k * 4096) | 0x3; // ...map the first 4MB of memory into the page table...
        kernelpagedir[k] = 0;           // ...and clear the page directory entries
    }

    // Fills the addresses 0...4MB and 3072MB...3076MB of the page directory
    // with the same page table

    kernelpagedir[0] = (unsigned long)lowpagetablePtr | 0x3;
    kernelpagedir[768] = (unsigned long)lowpagetablePtr | 0x3;

    write_page_directory((address_t)kernelpagedirPtr);
    enable_paging();
}

typedef void (*ctorfn)();
extern ctorfn* start_ctors; // defined by linker
extern ctorfn* end_ctors;

/* This entry point is called from asm loader */
void kernel_entry(multiboot::header *h)
{
    // set up paging for higher-half kernel
    init_paging();
    // install gdt
    global_descriptor_table::init();

    // run static ctors
    for (ctorfn* ctor = start_ctors; ctor < end_ctors; ctor++)
        (*ctor)();

    // start actual kernel
    kconsole.clear();
    multiboot::self().set_header(h);
    kernel.run(); /* does not return */
}

static inline void* placement_alloc(size_t size)
{
    uint32_t tmp = kmemmgr.get_placement_address();
    kmemmgr.set_placement_address(tmp+size);
    return (void *)tmp;
}

/* SUPPORT C CODE */
void *malloc(size_t size)
{
    if (kmemmgr.is_heap_initialised())
    {
//         kconsole << "malloc " << (int)size << endl;
        return kmemmgr.malloc(size);
    }
    else
        return placement_alloc(size);
}

void free(void *p)
{
    if (kmemmgr.is_heap_initialised())
        kmemmgr.free(p);
}

void *realloc(void *p, size_t size)
{
    if (kmemmgr.is_heap_initialised())
        return kmemmgr.realloc(p, size);
    else
    {
        // guess a maximum possible size for in-place allocated object
        size_t old_size_guess = kmemmgr.get_placement_address() - (address_t)p;
        // in-place alloc in new place
        void *tmp = placement_alloc(size);
        size = min(old_size_guess, size);
        memutils::copy_memory(tmp, p, size);
        return tmp;
    }
}

// ** SUPPORT CODE ** FIXME: move operator new impl to g++support.cpp?

void* operator new(size_t size)
{
    return malloc(size);
}

/** In-place new */
void *operator new(size_t size, uint32_t place)
{
	UNUSED(size);
	return (void *)place;
}

//address_t *addr?
void *operator new(size_t size, bool pageAlign, uint32_t *addr)
{
    if (kmemmgr.is_heap_initialised())
	{
		return kmemmgr.malloc(size, pageAlign, addr);
	}
	else
	{
		if (pageAlign)
			kmemmgr.align_placement_address();
        void *tmp = placement_alloc(size);
		if (addr)
			*addr = (uint32_t)tmp;
		return tmp;
	}
}

/**
* Carbon-copy of operator new(uint32_t).
**/
void *operator new[](size_t size)
{
	return malloc(size);
}

/**
* Carbon-copy of operator new(uint32_t,uint32_t*,bool).
**/
void *operator new[](size_t size, bool page_align, uint32_t *addr)
{
    if (kmemmgr.is_heap_initialised())
	{
		return kmemmgr.malloc(size, page_align, addr);
	}
	else
	{
        if (page_align)
            kmemmgr.align_placement_address();
        void *tmp = placement_alloc(size);
        if (addr)
            *addr = (uint32_t)tmp;
        return tmp;
	}
}

void  operator delete(void *p)
{
    if (kmemmgr.is_heap_initialised())
        kmemmgr.free(p);
}

void  operator delete[](void *p)
{
    if (kmemmgr.is_heap_initialised())
        kmemmgr.free(p);
}

// We encountered a massive problem and have to stop.
void panic(const char *message, const char *file, uint32_t line)
{
	disable_interrupts();

	kconsole.set_attr(RED, YELLOW);
	kconsole.print("PANIC (%s) at %s:%d\n", message, file, (int)line);
	kernel.print_backtrace();

	// Halt by going into an infinite loop.
	while(1) {}
}

// An assertion failed, and we have to panic.
void panic_assert(const char *desc, const char *file, uint32_t line)
{
	disable_interrupts();

	kconsole.set_attr(WHITE, RED);
	kconsole.print("ASSERTION-FAILED(%s) at %s:%d\n", desc, file, (int)line);
	kernel.print_backtrace();

	// Halt by going into an infinite loop.
	while(1) {}
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
