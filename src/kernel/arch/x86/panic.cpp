//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2010, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "default_console.h"
#include "panic.h"
#include "cpu.h"

void panic(const char* message, const char* file, uint32_t line)
{
    x86_cpu_t::disable_interrupts();

    kconsole.set_attr(RED, YELLOW);
    kconsole.print("PANIC! %s at %s:%d\n", message, file, (int)line);

    halt();
}

void panic_assert(const char* desc, const char* file, uint32_t line)
{
    x86_cpu_t::disable_interrupts();

    kconsole.set_attr(WHITE, RED);
    kconsole.print("ASSERTION FAILED! %s at %s:%d\n", desc, file, (int)line);

    halt();
}

void halt()
{
    x86_cpu_t::disable_interrupts();
    // Halt by going into an infinite loop.
    while(1) {}
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
