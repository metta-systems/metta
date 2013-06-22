//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2012, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "types.h"
#include "macros.h"

namespace logger {

class function_scope
{
    const char* name;
public:
    function_scope(const char *fn);
    ~function_scope();
};

}
/**
 * The kernel debugger.
 */
class debugger_t
{
public:
    /**
     * Dump @c size bytes from a memory region starting at virtual address @c start.
     */
    static void dump_memory(address_t start, size_t size);

    /**
     * Given a stack base pointer, follow it, return the next stack base
     * pointer and also return the instruction pointer it returned to.
     */
    static address_t backtrace(address_t base_pointer, address_t& return_address);

    /**
     * Given the current stack, follow @c n backtraces and return the
     * return address found there.
     */
    static address_t backtrace(int n);

    /**
     * Print a full backtrace from the start location @c eip and following the stack frames starting at @c base_pointer.
     * (Or, if @c n is specified, up to @c n stack frames.)
     */
    static void print_backtrace(address_t base_pointer = 0, address_t eip = 0, int n = 0);

    /**
     * Prints first @c n words from the stack
     */
    static void print_stacktrace(unsigned int n = 64);

    /**
     * Prints [checkpoint] followed by checkpoint name from @c str and then waits for Enter keypress.
     */
    static void checkpoint(const char* str);
    
    /**
     * Trigger a cpu breakpoint. Will cause a magic trap under bochs.
     */
    static void breakpoint();
};

// Helpers for easier debugging in Bochs
#if BOCHS_IO_HACKS
#include "cpu.h"

/**
 * Outputs a character to the debug console.
 */
inline void bochs_console_print_char(int c)
{
    x86_cpu_t::outb(0xe9, c);
}

/**
 * Outputs a string to the debug console.
 */
inline void bochs_console_print_str(const char* str)
{
    char *b = (char *)str;
    while (*b)
        bochs_console_print_char(*b++);
}

/**
 * Stops simulation and breaks into the debug console.
 * This is an alternative to bochs_magic_trap.
 */
inline void bochs_break()
{
    x86_cpu_t::outw(0x8A00,0x8A00);
    x86_cpu_t::outw(0x8A00,0x8AE0);
}

void bochs_magic_trap()  ALWAYS_INLINE;

/**
 * Traps into debug console (add "magic_break: enabled=1" to bochs config).
 */
inline void bochs_magic_trap()
{
    asm volatile("xchg %bx, %bx");
}

/**
 * Monitor memory area from start to end for writes and reads.
 */
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
#define bochs_console_print_str(s)
#define bochs_break()
#define bochs_magic_trap()
#define bochs_add_watch_region(start,end)
#endif
