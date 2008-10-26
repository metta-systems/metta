//
// Copyright 2007 - 2008, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "Globals.h"
#include "Registers.h"
#include "DefaultConsole.h"
#include "Kernel.h"
#include "Multiboot.h"
#include "ElfParser.h"
#include "MemoryManager.h"
#include "InterruptDescriptorTable.h"

/* Global objects FIXME: use singletons instead? */
class kernel kernel;
multiboot_t multiboot;
ElfParser kernelElfParser;
MemoryManager memoryManager;
InterruptDescriptorTable interruptsTable;

/* This entry point is called from loader */
void kernel_entry(multiboot_header_t *multiboot_header)
{
	kconsole.clear();
	multiboot = multiboot_t(multiboot_header);
	kernel.run(); /* does not return */
}

// ** SUPPORT CODE ** FIXME: move operator new impl to g++support.cpp?

void* operator new(size_t size)
{
	if (memoryManager.isHeapInitialised())
	{
		return memoryManager.malloc(size);
	}
	else
	{
		uint32_t tmp = memoryManager.getPlacementAddress();
		memoryManager.setPlacementAddress(tmp+size);
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
	if (memoryManager.isHeapInitialised())
	{
		return memoryManager.malloc(size, pageAlign, addr);
	}
	else
	{
		if (pageAlign)
			memoryManager.alignPlacementAddress();
		uint32_t tmp = memoryManager.getPlacementAddress();
		if (addr)
			*addr = tmp;
		memoryManager.setPlacementAddress(tmp+size);
		return (void *)tmp;
	}
}

/**
Carbon-copy of operator new(uint32_t).
**/
void *operator new[](size_t size)
{
	return operator new(size);
}

/**
Carbon-copy of operator new(uint32_t,uint32_t*,bool).
**/
void *operator new[](size_t size, bool pageAlign, uint32_t *addr)
{
	if (memoryManager.isHeapInitialised())
	{
		return memoryManager.malloc(size, pageAlign, addr);
	}
	else
	{
		if (pageAlign) {memoryManager.alignPlacementAddress();}
		uint32_t tmp = memoryManager.getPlacementAddress();
		if (addr) {*addr = tmp;}
		memoryManager.setPlacementAddress(tmp+size);
		return (void *)tmp;
	}
}

void  operator delete(void *p)
{
	memoryManager.free(p);
}

void  operator delete[](void *p)
{
	memoryManager.free(p);
}

// We encountered a massive problem and have to stop.
void panic(const char *message, const char *file, uint32_t line)
{
	disableInterrupts();

	kconsole.set_attr(RED, YELLOW);
	kconsole.print("PANIC (%s) at %s:%d\n", message, file, line);
	kernel.print_backtrace();

	// Halt by going into an infinite loop.
	while(1) {}
}

// An assertion failed, and we have to panic.
void panic_assert(const char *desc, const char *file, uint32_t line)
{
	disableInterrupts();

	kconsole.set_attr(WHITE, RED);
	kconsole.print("ASSERTION-FAILED(%s) at %s:%d\n", desc, file, line);
	kernel.print_backtrace();

	// Halt by going into an infinite loop.
	while(1) {}
}
// kate: indent-width 4; replace-tabs on;
// vi:set ts=4:set expandtab=on:
