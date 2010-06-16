#pragma once

#include "types.h"

// continuation (it records the state of the running computation at the point where it takes off).
class continuation_t
{
    // continuation flags
    static const int F_CPU_VALID = 1; // CPU context is valid.
    static const int F_FPU_VALID = 2; // FPU context is valid.

public:
    struct gpregs_t {
        uint32_t eax, ebx, ecx, edx, esi, edi, ebp;
        uint32_t eip;
        uint32_t eflags;
        uint32_t esp;
    };

    continuation_t() : flags(0) {}

    void set_entry(uint32_t entry) { regs.eip = entry; }
    void set_gpregs(gpregs_t reg) { regs = reg; flags |= F_CPU_VALID; }

    void activate();

private:
    gpregs_t regs;
    uint32_t pervasives;
    uint32_t flags;
    uint32_t fpregs[27]; // FPU registers
    uint32_t pad[25]; // use it for e.g. some words on stack too?
};
