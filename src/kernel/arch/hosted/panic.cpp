//
// Part of Metta OS. Check https://atta-metta.net for latest version.
//
// Copyright 2007 - 2017, Stanislav Karchebnyy <berkus@atta-metta.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "panic.h"
#include "debugger.h"
#include "default_console.h"

void panic(const char* message, const char* file, uint32_t line)
{
    kconsole << "PANIC! " << message << " at " << file << ":" << (int)line << endl;
    // debugger_t::print_backtrace(0, 0, 20);
    halt();
}

void panic_assert(const char* desc, const char* file, uint32_t line)
{
    kconsole << "ASSERTION FAILED! " << desc << " at " << file << ":" << (int)line << endl;
    // debugger_t::print_backtrace(0, 0, 20);
    halt();
}

void halt()
{
    // Halt by going into an infinite loop.
    while(1) {}
}
