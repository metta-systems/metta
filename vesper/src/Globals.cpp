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
#include "interrupt_descriptor_table.h"

namespace metta {
namespace kernel {

/* Global objects FIXME: use singletons instead? */
class kernel kernel;
class multiboot multiboot;
elf_parser kernel_elf_parser;
class memory_manager memory_manager;
interrupt_descriptor_table interrupts_table;

}
}

using namespace metta::kernel;

/* This entry point is called from loader */
void kernel_entry(multiboot::header *multiboot_header)
{
	kconsole.clear();
	multiboot = multiboot::multiboot(multiboot_header);
	kernel.run(); /* does not return */
}

// ** SUPPORT CODE ** FIXME: move operator new impl to g++support.cpp?

void* operator new(size_t size)
{
	if (memory_manager.is_heap_initialised())
	{
		return memory_manager.malloc(size);
	}
	else
	{
		uint32_t tmp = memory_manager.get_placement_address();
		memory_manager.set_placement_address(tmp+size);
		return (void *)tmp;
	}
}

void *operator new(size_t size, uint32_t place)
{
	UNUSED(size);
	return (void *)place;
}

void *operator new(size_t size, bool pageAlign, uint32_t *addr)
{
    if (memory_manager.is_heap_initialised())
	{
		return memory_manager.malloc(size, pageAlign, addr);
	}
	else
	{
		if (pageAlign)
			memory_manager.align_placement_address();
		uint32_t tmp = memory_manager.get_placement_address();
		if (addr)
			*addr = tmp;
		memory_manager.set_placement_address(tmp+size);
		return (void *)tmp;
	}
}

/**
* Carbon-copy of operator new(uint32_t).
**/
void *operator new[](size_t size)
{
	return operator new(size);
}

/**
* Carbon-copy of operator new(uint32_t,uint32_t*,bool).
**/
void *operator new[](size_t size, bool pageAlign, uint32_t *addr)
{
    if (memory_manager.is_heap_initialised())
	{
		return memory_manager.malloc(size, pageAlign, addr);
	}
	else
	{
		if (pageAlign) {memory_manager.align_placement_address();}
		uint32_t tmp = memory_manager.get_placement_address();
		if (addr) {*addr = tmp;}
		memory_manager.set_placement_address(tmp+size);
		return (void *)tmp;
	}
}

void  operator delete(void *p)
{
	memory_manager.free(p);
}

void  operator delete[](void *p)
{
	memory_manager.free(p);
}

// We encountered a massive problem and have to stop.
void panic(const char *message, const char *file, uint32_t line)
{
	disable_interrupts();

	kconsole.set_attr(RED, YELLOW);
	kconsole.print("PANIC (%s) at %s:%d\n", message, file, line);
	kernel.print_backtrace();

	// Halt by going into an infinite loop.
	while(1) {}
}

// An assertion failed, and we have to panic.
void panic_assert(const char *desc, const char *file, uint32_t line)
{
	disable_interrupts();

	kconsole.set_attr(WHITE, RED);
	kconsole.print("ASSERTION-FAILED(%s) at %s:%d\n", desc, file, line);
	kernel.print_backtrace();

	// Halt by going into an infinite loop.
	while(1) {}
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
