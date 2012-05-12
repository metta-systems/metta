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
#include "pervasives_v1_interface.h"
#include "infopage.h"

// continuation (it records the state of the running computation at the point where it takes off).
class continuation_t
{
    // continuation flags
    static const int F_CPU_VALID = 1; // CPU context is valid.
    static const int F_FPU_VALID = 2; // FPU context is valid.
    static const int F_PERV_VALID = 4; // Pervasives pointer is valid.

public:
    struct gpregs_t {
        uint32_t eax, ebx, ecx, edx, esi, edi, ebp;
        uint32_t eip;
        uint32_t eflags;
        uint32_t esp;
    };

    continuation_t() : flags(0) {}

    void set_entry(uint32_t entry, uint32_t new_cs, uint32_t new_ds)
    {
        regs.eip = entry;
        cs = new_cs;
        ds = new_ds;
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

extern "C" void asm_activate(continuation_t::gpregs_t* gpregs, uint32_t cs, uint32_t ds);

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
        asm volatile ("frstor %0" :: "m"(*fpregs));
    }
    // restore CPU registers
    // set cs:ds:es, flags and stack pointer
    // iret
    asm_activate(&regs, cs, ds);
}
