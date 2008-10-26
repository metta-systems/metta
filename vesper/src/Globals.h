//
// Copyright 2007 - 2008, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "Types.h"
#include "Macros.h"
#include "string.h"

extern class kernel kernel;
extern class multiboot_t multiboot;
extern class elf_parser kernelElfParser;
extern class MemoryManager memoryManager;
extern class InterruptDescriptorTable interruptsTable;

extern "C" void kernel_entry(class multiboot_header_t *mh) NORETURN;

void *operator new(size_t size);
void *operator new(size_t size, uint32_t place);
void *operator new(size_t size, bool pageAlign, uint32_t *physAddr=NULL);
void *operator new[](size_t size);
void *operator new[](size_t size, bool pageAlign, uint32_t *physAddr=NULL);
void  operator delete(void *p);
void  operator delete[](void *p);
// kate: indent-width 4; replace-tabs on;
// vi:set ts=4:set expandtab=on:
