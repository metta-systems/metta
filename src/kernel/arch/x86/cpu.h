//
// Part of Metta OS. Check http://atta-metta.net for latest version.
//
// Copyright 2007 - 2012, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "types.h"
#include "macros.h"
#include "cpu_flags.h"
#include "cpu_information.h"
#include "ia32.h"

/**
 * Intel x86 CPU related operations.
 */
class x86_cpu_t
{
public:
    /**
     * Set a flag in CR0.
     */
    static inline void cr0_set_flag(uint32_t flag) ALWAYS_INLINE
    {
        uint32_t dummy;
        asm volatile ("movl %%cr0, %0\n" : "=r"(dummy));
        dummy |= flag;
        asm volatile ("movl %0, %%cr0\n" :: "r"(dummy));
    }

    /**
     * Set a flag in CR4.
     */
    static inline void cr4_set_flag(uint32_t flag) ALWAYS_INLINE
    {
        uint32_t dummy;
        asm volatile ("movl %%cr4, %0\n" : "=r"(dummy));
        dummy |= flag;
        asm volatile ("movl %0, %%cr4\n" :: "r"(dummy));
    }

    /**
     * Write a byte out to the specified port.
     */
    static inline void outb(uint16_t port, uint8_t value) ALWAYS_INLINE
    {
        //("a" puts value in eax, "dN" puts port in edx or uses 1-byte constant.)
        asm volatile ("outb %0, %1" :: "a" (value), "dN" (port));
    }

    /**
     * Write a word out to the specified port.
     */
    static inline void outw(uint16_t port, uint16_t value) ALWAYS_INLINE
    {
        asm volatile ("outw %0, %1" :: "a" (value), "dN" (port));
    }

    /**
     * Write a dword out to the specified port.
     */
    static inline void outl(uint16_t port, uint32_t value) ALWAYS_INLINE
    {
        asm volatile ("outl %0, %1" :: "a" (value), "dN" (port));
    }

    /**
     * Read a byte in from the specified port.
     */
    static inline uint8_t inb(uint16_t port) ALWAYS_INLINE
    {
        uint8_t ret;
        asm volatile("inb %1, %0" : "=a" (ret) : "dN" (port));
        return ret;
    }

    /**
     * Read a word in from the specified port.
     */
    static inline uint16_t inw(uint16_t port) ALWAYS_INLINE
    {
        uint16_t ret;
        asm volatile ("inw %1, %0" : "=a" (ret) : "dN" (port));
        return ret;
    }

    /**
     * Read a dword in from the specified port.
     */
    static inline uint32_t inl(uint16_t port) ALWAYS_INLINE
    {
        uint32_t ret;
        asm volatile ("inl %1, %0" : "=a" (ret) : "dN" (port));
        return ret;
    }

    /**
     * Read internal CPU 64-bit clock (timestamp counter).
     */
    static inline uint64_t read_tsc() ALWAYS_INLINE
    {
        uint64_t ret;
        asm volatile("rdtsc" : "=A"(ret));
        return ret;
    }

    /**
     * Read machine-specific register.
     */
    static inline uint64_t read_msr(uint32_t index) ALWAYS_INLINE
    {
        uint64_t value;
        asm volatile("rdmsr" : "=A" (value) : "c" (index));
        return value;
    }

    /**
     * Write machine-specific register.
     */
    static inline void write_msr(uint32_t index, uint64_t value) ALWAYS_INLINE
    {
        asm volatile("wrmsr" :: "A" (value), "c" (index));
    }

    /**
     * Enable external interrupts.
     */
    static inline void enable_interrupts() ALWAYS_INLINE
    {
        asm volatile ("sti");
    }

    /**
     * Disable external interrupts.
     */
    static inline void disable_interrupts() ALWAYS_INLINE
    {
        asm volatile ("cli");
    }

    //! id of current processor.
    static inline cpu_id_t id() { return 0; }

    static inline cpu_information_t& current_cpu() { return cpu_information; }

    static inline bool has_cpuid()
    {
        /* Iff bit 21 in EFLAGS can be set the CPU supports the CPUID instruction */
        uint32_t eflags;
        asm volatile (
        // Save EFLAGS to the stack
        "pushfl                 \n"
        // Set bit 21 in EFLAGS image on stack
        "orl     %1,(%%esp)     \n"
        // Restore EFLAGS from stack.
        "popfl                  \n"
        // If supported, this has set bit 21
        // Save EFLAGS on stack to see if bit 21 was set or not
        "pushfl                 \n"
        // Move EFLAGS image to register for inspection
        "pop     %0             \n"
        : "=a" (eflags)
        : "i" (X86_FLAGS_ID)
        );
        return (eflags & X86_FLAGS_ID);
    }

    static inline uint32_t features()
    {
        if (has_cpuid())
        {
            uint32_t features, dummy;
            cpuid(1, &dummy, &dummy, &dummy, &features);
            return features;
        }
        else
        {
            /*
             * If there is no CPUID instruction we just fabricate the
             * appropriate feature word.  Currently we only support
             * i486DX+ and therefore assume the FPU to be present
             */
            return X86_32_FEAT_FPU;
        }
    }

    static inline void cpuid(uint32_t func, uint32_t* eax, uint32_t* ebx, uint32_t* ecx, uint32_t* edx) ALWAYS_INLINE
    {
        asm volatile ("cpuid"
                      : "=a" (*eax), "=b" (*ebx), "=c" (*ecx), "=d" (*edx)
                      : "a" (func));
    }

    /* Clear TS bit so we don't trap on FPU instructions. */
    static inline void enable_fpu() ALWAYS_INLINE
    {
        asm volatile ("fninit");
        asm volatile ("clts");
    }

    static inline void init_cache()
    {
        asm volatile ("wbinvd\n"                    /* Flush cache */
                      "movl    %cr0, %eax\n"
                      "andl    $0x9fffffff, %eax\n" /* Clear cache disable bits */
                      "movl    %eax, %cr0");
    }

    static inline void enable_alignment_checks() ALWAYS_INLINE
    {
        cr0_set_flag(IA32_CR0_AM);
    }

    /* setup to read DCstall cycles + inst retired */
    static inline void init_pmctr()
    {
        write_msr(X86_MSR_PMCTR0, 0);
        write_msr(X86_MSR_PMCTR1, 0);
        write_msr(X86_MSR_EVSEL1, 0x30048); // DCU wait cycles
        write_msr(X86_MSR_EVSEL0, 0x4300C0); // Insts retired + EN + OS + USR
    }

    static inline void enable_user_pmctr() ALWAYS_INLINE
    {
        cr4_set_flag(IA32_CR4_PCE); /* enable read from user land */
    }

private:
    static cpu_information_t cpu_information;
};
