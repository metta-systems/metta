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
};
