//
// Copyright 2007 - 2008, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "Globals.h"
#include "Registers.h"
#include "DefaultConsole.h"
#include "Kernel.h"
#include "Multiboot.h"
#include "ElfParser.h"
#include "MemoryManager.h"
#include "InterruptDescriptorTable.h"

namespace metta {
namespace kernel {

/* Global objects FIXME: use singletons instead? */
class kernel kernel;
class multiboot multiboot;
elf_parser kernelElfParser;
MemoryManager memory_manager;
interrupt_descriptor_table interruptsTable;

}
}

using namespace metta::kernel;

/* This entry point is called from loader */
void kernel_entry(multiboot_header *multiboot_header)
{
	kconsole.clear();
	multiboot = multiboot::multiboot(multiboot_header);
	kernel.run(); /* does not return */
}

// ** SUPPORT CODE ** FIXME: move operator new impl to g++support.cpp?

void* operator new(size_t size)
{
	if (memory_manager.isHeapInitialised())
	{
		return memory_manager.malloc(size);
	}
	else
	{
		uint32_t tmp = memory_manager.getPlacementAddress();
		memory_manager.setPlacementAddress(tmp+size);
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
	if (memory_manager.isHeapInitialised())
	{
		return memory_manager.malloc(size, pageAlign, addr);
	}
	else
	{
		if (pageAlign)
			memory_manager.alignPlacementAddress();
		uint32_t tmp = memory_manager.getPlacementAddress();
		if (addr)
			*addr = tmp;
		memory_manager.setPlacementAddress(tmp+size);
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
	if (memory_manager.isHeapInitialised())
	{
		return memory_manager.malloc(size, pageAlign, addr);
	}
	else
	{
		if (pageAlign) {memory_manager.alignPlacementAddress();}
		uint32_t tmp = memory_manager.getPlacementAddress();
		if (addr) {*addr = tmp;}
		memory_manager.setPlacementAddress(tmp+size);
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
// vi:set ts=4:set expandtab=on:
