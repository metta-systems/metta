//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "types.h"
#include "macros.h"
#include "multiboot.h"

namespace metta {
namespace kernel {

extern class kernel kernel;
extern class multiboot multiboot;
extern class elf_parser kernel_elf_parser;
extern class interrupt_descriptor_table interrupts_table;

}
}

extern "C"
{
    void kernel_entry(metta::kernel::multiboot::header *mh) NORETURN;

    void *malloc(size_t size);
    void free(void *p);
    void *realloc(void *p, size_t size);
}

// use some tmpl header
template <typename T>
T min(T a, T b)
{
    return (a < b ? a : b);
}

template <typename T>
T max(T a, T b)
{
    return (a > b ? a : b);
}

void *operator new(size_t size);
void *operator new(size_t size, uint32_t place);
void *operator new(size_t size, bool page_align, uint32_t *phys_addr = NULL);
void *operator new[](size_t size);
void *operator new[](size_t size, bool page_align, uint32_t *phys_addr = NULL);
void  operator delete(void *p);
void  operator delete[](void *p);

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
