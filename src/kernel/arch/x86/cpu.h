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
#include "macros.h"
#include "mmu.h"
#include "cpu_flags.h"
#include "cpu_information.h"

class x86_cpu_t
{
public:
    /*!
     * Write a byte out to the specified port.
     */
    static inline void outb(uint16_t port, uint8_t value) ALWAYS_INLINE
    {
        //("a" puts value in eax, "dN" puts port in edx or uses 1-byte constant.)
        asm volatile ("outb %0, %1" :: "a" (value), "dN" (port));
    }

    /*!
     * Write a word out to the specified port.
     */
    static inline void outw(uint16_t port, uint16_t value)
    {
        asm volatile ("outw %0, %1" :: "a" (value), "dN" (port));
    }

    /*!
     * Read a byte in from the specified port.
     */
    static inline uint8_t inb(uint16_t port)
    {
        uint8_t ret;
        asm volatile("inb %1, %0" : "=a" (ret) : "dN" (port));
        return ret;
    }

    /*!
     * Read a word in from the specified port.
     */
    static inline uint16_t inw(uint16_t port)
    {
        uint16_t ret;
        asm volatile ("inw %1, %0" : "=a" (ret) : "dN" (port));
        return ret;
    }

    /*!
     * Read internal CPU 64-bit clock (timestamp counter).
     */
    static inline uint64_t rdtsc()
    {
        uint64_t ret;
        asm volatile("rdtsc" : "=A"(ret));
        return ret;
    }

    /*!
     * Write machine-specific register.
     */
    static inline void wrmsr(uint32_t index, uint64_t value)
    {
        asm volatile("wrmsr" :: "A" (value), "c" (index));
    }

    /*!
     * Enable external interrupts.
     */
    static inline void enable_interrupts() ALWAYS_INLINE
    {
        asm volatile ("sti");
    }

    /*!
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

    static inline void cpuid(uint32_t func, uint32_t* eax, uint32_t* ebx, uint32_t* ecx, uint32_t* edx)
    {
        asm volatile ("cpuid"
                      : "=a" (*eax), "=b" (*ebx), "=c" (*ecx), "=d" (*edx)
                      : "a" (func));
    }

    /* Clear TS bit so we don't trap on FPU instructions. */
    static inline void enable_fpu()
    {
        asm volatile ("clts");
    }

    static inline void init_cache()
    {
        asm volatile ("wbinvd\n"                    /* Flush cache */
                      "movl    %cr0, %eax\n"
                      "andl    $0x9fffffff, %eax\n" /* Clear cache disable bits */
                      "movl    %eax, %cr0");
    }

    static inline void enable_alignment_checks()
    {
        ia32_cr0_set(IA32_CR0_AM);
    }

    /* setup to read DCstall cycles + inst retired */
    static inline void init_pmctr()
    {
        wrmsr(X86_MSR_PMCTR0, 0);
        wrmsr(X86_MSR_PMCTR1, 0);
        wrmsr(X86_MSR_EVSEL1, 0x30048); // DCU wait cycles
        wrmsr(X86_MSR_EVSEL0, 0x4300C0); // Insts retired + EN + OS + USR
    }

    static inline void enable_user_pmctr()
    {
        ia32_cr4_set(IA32_CR4_PCE); /* enable read from user land */
    }

private:
    static cpu_information_t cpu_information;
};

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
