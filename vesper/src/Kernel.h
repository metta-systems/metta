//
// Copyright 2007 - 2008, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "types.h"
#include "macros.h"
#include "string.h"

namespace metta {
namespace kernel {

class kernel
{
public:
    /**
    * Boot the kernel.
    **/
    void run() NORETURN;

    /**
    * Dump @p size bytes from a memory region starting at virtual address @p start.
    **/
    static void dump_memory(address_t start, size_t size);

    /**
    * Given a stack base pointer, follow it, return the next stack base
    * pointer and also return the instruction pointer it returned to.
    **/
    static address_t backtrace(address_t base_pointer, address_t& return_address);

    /**
    * Given the current stack, follow 'n' backtraces and return the
    * return address found there.
    **/
    static address_t backtrace(int n);

    /**
    * memset - Fill a region of memory with the given value
    * @p dest Pointer to the start of the area.
    * @p value The byte to fill the area with
    * @p count The size of the area.
    *
    * Do not use memset() to access IO space, use memset_io() instead.
    **/
    inline static void* set_memory(void* dest, int value, size_t count)
    {
        return memset(dest, value, count);
    }

    /**
    * memcpy - Copy one area of memory to another
    * @p dest Where to copy to
    * @p src Where to copy from
    * @p count The size of the area.
    *
    * You should not use this function to access IO space, use memcpy_toio()
    * or memcpy_fromio() instead.
    **/
    inline static void* copy_memory(void* dest, const void* src, size_t count)
    {
        return memcpy(dest, src, count);
    }

    /**
    * memmove - Copy one area of memory to another
    * @p dest Where to copy to
    * @p src Where to copy from
    * @p count The size of the area.
    *
    * Unlike memcpy(), memmove() copes with overlapping areas.
    **/
    inline static void* move_memory(void* dest, const void* src, size_t count)
    {
        return memmove(dest, src, count);
    }

    /**
    * Print a full backtrace from the current location. (Or, if @p n is specified,
    * up to n stack frames.
    **/
    void print_backtrace(address_t base_pointer = 0, int n = 0);

    /**
    * Prints first @p n words from the stack
    **/
    void print_stacktrace(unsigned int n = 64);

private:
    void relocate_placement_address();
};

}
}

// kate: indent-width 4; replace-tabs on;
// vi:set ts=4:set expandtab=on:
