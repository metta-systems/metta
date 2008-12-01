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
#include "Multiboot.h"

namespace metta {
namespace kernel {

extern class kernel kernel;
extern class multiboot multiboot;
extern class elf_parser kernel_elf_parser;
extern class memory_manager memory_manager;
extern class interrupt_descriptor_table interrupts_table;

}
}

extern "C" void kernel_entry(metta::kernel::multiboot_header *mh) NORETURN;

void *operator new(size_t size);
void *operator new(size_t size, uint32_t place);
void *operator new(size_t size, bool page_align, uint32_t *phys_addr = NULL);
void *operator new[](size_t size);
void *operator new[](size_t size, bool page_align, uint32_t *phys_addr = NULL);
void  operator delete(void *p);
void  operator delete[](void *p);

// kate: indent-width 4; replace-tabs on;
// vi:set ts=4:set expandtab=on:
