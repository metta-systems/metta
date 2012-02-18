//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2011, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "types.h"
#include "segs.h"
// C header to support continuations:
#include <setjmp.h>

// continuation (it records the state of the running computation at the point where it takes off).
class continuation_t
{
    // continuation flags
    static const int F_CPU_VALID = 1; // CPU context is valid.
    static const int F_FPU_VALID = 2; // FPU context is valid.
    static const int F_PERV_VALID = 4; // Pervasives pointer is valid.

public:
    typedef jmp_buf gpregs_t;

    continuation_t() : flags(0) {}

    void set_entry(uint32_t entry, uint32_t new_cs = KERNEL_CS, uint32_t new_ds = KERNEL_DS)
    {
        setjmp(entry, regs);
//        regs.eip = entry;
        // Ignore CS and DS..
    }
    void set_gpregs(gpregs_t reg) { regs = reg; flags |= F_CPU_VALID; }

    void activate();

private:
    gpregs_t regs;
    pervasives_v1::rec* pervasives;
    uint32_t flags;
    uint32_t cs, ds;
    uint32_t fpregs[27]; // FPU registers
    uint32_t pad[23]; // use it for e.g. some words on stack too?
};

// A privileged method to activate (throw) a continuation.
// Ideally, the FPU stuff should be initialiazed only if process hits FPU exception by using FPU commands after
// activation. Saves some switch time.
void continuation_t::activate()
{
    // restore pervasives pointer
    if (flags & F_PERV_VALID)
    {
        INFO_PAGE.pervasives = pervasives;
    }
    // restore FPU registers
    if (flags & F_FPU_VALID)
    {
        // Not supported in hosted implementation.
//        asm volatile ("frstor %0" :: "m"(*fpregs));
    }
    // restore CPU registers
    longjmp(regs);
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
