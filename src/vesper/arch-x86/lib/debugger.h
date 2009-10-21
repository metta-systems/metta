//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "types.h"

// The kernel debugger.
class debugger_t
{
public:
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
    * Print a full backtrace from the current location. (Or, if @p n is specified,
    * up to n stack frames.
    **/
    static void print_backtrace(address_t base_pointer = 0, int n = 0);

    /**
    * Prints first @p n words from the stack
    **/
    static void print_stacktrace(unsigned int n = 64);

    /**
     * Prints [checkpoint] followed by checkpoint name from @p str and then waits for Enter keypress.
     */
    static void checkpoint(const char* str);
};

// Helpers for easier debugging in Bochs
#if BOCHS_IO_HACKS
#include "cpu.h"

//outputs a character to the debug console
inline void bochs_console_print_char(int c)
{
    x86_cpu_t::outb(0xe9, c);
}

//stops simulation and breaks into the debug console
inline void bochs_break()
{
    x86_cpu_t::outw(0x8A00,0x8A00);
    x86_cpu_t::outw(0x8A00,0x08AE0);
}

//traps into debug console (add "magic_break: enabled=1" to bochs config)
inline void bochs_magic_trap()
{
    asm volatile("xchg %bx, %bx");
}

//monitor memory area from start to end for writes and reads
inline void bochs_add_watch_region(address_t start, address_t end)
{
    x86_cpu_t::outw(0x8A00,0x8A00);
    x86_cpu_t::outw(0x8A00,0x8A01);
    x86_cpu_t::outw(0x8A01,(start>>16)&0xffff);
    x86_cpu_t::outw(0x8A01,start&0xffff);
    x86_cpu_t::outw(0x8A00,0x8A02);
    x86_cpu_t::outw(0x8A01,(end>>16)&0xffff);
    x86_cpu_t::outw(0x8A01,end&0xffff);
    x86_cpu_t::outw(0x8A00,0x8A80);
}

#else
#define bochs_console_print_char(c)
#define bochs_break()
#define bochs_magic_trap()
#define bochs_add_watch_region(start,end)
#endif

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
