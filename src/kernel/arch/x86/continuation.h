#pragma once

#include "types.h"
#include "segs.h"

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

    void set_entry(uint32_t entry, uint32_t new_cs = KERNEL_CS, uint32_t new_ds = KERNEL_DS)
    {
        regs.eip = entry;
        cs = new_cs;
        ds = new_ds;
    }
    void set_gpregs(gpregs_t reg) { regs = reg; flags |= F_CPU_VALID; }

    void activate();

private:
    gpregs_t regs;
    uint32_t pervasives;
    uint32_t flags;
    uint32_t cs, ds;
    uint32_t fpregs[27]; // FPU registers
    uint32_t pad[23]; // use it for e.g. some words on stack too?
};

extern void asm_activate(continuation_t::gpregs_t* gpregs, uint32_t cs, uint32_t ds);

// A privileged method to activate (throw) a continuation.
// Ideally, the FPU stuff should be initialiazed only if process hits FPU exception by using FPU commands after
// activation. Saves some switch time.
void continuation_t::activate()
{
    // restore pervasives pointer
    if (flags & F_PERV_VALID)
    {
        INFO_PAGE.pervasives = (void*)pervasives;
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
