//
// Minimal operator new/delete implementation.
//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2010, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "new.h"
#include "macros.h"
#include "memory.h"
#include "memutils.h"
#include "default_console.h"
#include "debugger.h"

// Compliant functions for STL allocator (TODO: replace stl malloc_alloc with other default allocator)
extern "C" void* malloc(size_t size)
{
    kconsole << RED << "malloc(" << size << ") -> ";
    debugger_t::print_backtrace();
    halt();
    return operator new(size, false, NULL);
}

extern "C" void free(void*p)
{
    kconsole << RED << "free(" << p << ") -> ";
    debugger_t::print_backtrace();
    halt();
}

// we assume that stl uses realloc only to grow storage
extern "C" void *realloc(void *ptr, size_t size)
{
    kconsole << RED << "realloc(" << ptr << ", " << size << ") -> ";
    debugger_t::print_backtrace();
    halt();
    void* ptr2 = malloc(size);
    memutils::copy_memory(ptr2, ptr, size); // may fail since new size is assumed to be larger than the old.
    free(ptr);
    return ptr2;
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
