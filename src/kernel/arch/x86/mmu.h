//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
/*********************************************************************
 *
 * Copyright (C) 2002-2003,  Karlsruhe University
 *
 * File path:     arch/ia32/mmu.h
 * Description: Methods for managing the IA-32 MMU
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $Id: mmu.h,v 1.8 2003/09/24 19:04:27 skoglund Exp $
 *
 ********************************************************************/
#pragma once

#include "types.h"
#include "ia32.h"
#include "cpu.h"
// #include "x86_protection_domain.h"

/**
 * MMU control for Intel 32 bit.
 */
class ia32_mmu_t
{
public:
    static inline void flush_page_directory(bool global = false);
    static inline void flush_page_directory_entry(address_t addr);
    static inline void flush_page_directory_entry(void* addr)
    {
        flush_page_directory_entry(reinterpret_cast<address_t>(addr));
    }
    static inline void enable_2mb_pages();
    static inline void enable_4mb_pages();
    static inline void enable_global_pages();
    static inline void enable_paged_mode();
    static inline void disable_paged_mode();
    static inline bool paged_mode_enabled();
    static inline address_t get_pagefault_address(void);
    static inline physical_address_t get_active_pagetable(void);
    static inline void set_active_pagetable(physical_address_t page_dir_physical);
//     static void set_active_pagetable(x86_protection_domain_t& pdom);
};

/**
 * Flushes the tlb
 *
 * @param global specifies whether global TLB entries are also flushed
 */
inline void ia32_mmu_t::flush_page_directory(bool global)
{
    uint32_t dummy, dummy2;
    if (!global)
        asm volatile ("movl %%cr3, %0\n"
                    "movl %0, %%cr3\n"
                    : "=r"(dummy));
    else
        asm volatile ("movl %%cr4, %0\n"
                    "andl %2, %0\n"
                    "movl %0, %%cr4\n"
                    "movl %%cr3, %1\n"
                    "movl %1, %%cr3\n"
                    "orl  %3, %0\n"
                    "movl %0, %%cr4\n"
                    : "=r"(dummy), "=r"(dummy2)
                    : "i" (~IA32_CR4_PGE), "i" (IA32_CR4_PGE));
}

/**
 * Flushes the TLB entry for a linear address
 *
 * @param linear linear address
 */
inline void ia32_mmu_t::flush_page_directory_entry(address_t linear) ALWAYS_INLINE
{
    asm volatile ("invlpg (%0)\n" :: "r"(linear));
}

/**
 * Enables physical address extension (2M pages) support for IA32.
 * Necessary for x86_64 mode.
 */
inline void ia32_mmu_t::enable_2mb_pages() ALWAYS_INLINE
{
    x86_cpu_t::cr4_set_flag(IA32_CR4_PAE);
}

/**
 * Enables extended page size (4M) support for IA32.
 */
inline void ia32_mmu_t::enable_4mb_pages() ALWAYS_INLINE
{
    x86_cpu_t::cr4_set_flag(IA32_CR4_PSE);
}

/**
 * Enables global page support for IA32.
 */
inline void ia32_mmu_t::enable_global_pages() ALWAYS_INLINE
{
    x86_cpu_t::cr4_set_flag(IA32_CR4_PGE);
}

/**
 * Enables paged mode for IA32.
 */
inline void ia32_mmu_t::enable_paged_mode() ALWAYS_INLINE
{
    asm volatile ("mov %0, %%cr0\n" :: "r"(IA32_CR0_PG | IA32_CR0_WP | IA32_CR0_PE));
}

/**
 * Enables paged mode for IA32
 */
inline void ia32_mmu_t::disable_paged_mode() ALWAYS_INLINE
{
    asm volatile ("mov %0, %%cr0\n" :: "r"(IA32_CR0_WP | IA32_CR0_PE));
}

/**
 * Check if paging is enabled.
 */
inline bool ia32_mmu_t::paged_mode_enabled() ALWAYS_INLINE
{
    uint32_t cr0;
    asm volatile("movl %%cr0, %0\n" : "=r"(cr0));
    return (cr0 & IA32_CR0_PG) != 0;
}

/**
 * @returns the linear address of the last pagefault
 */
inline address_t ia32_mmu_t::get_pagefault_address() ALWAYS_INLINE
{
    uint32_t faulting_address;
    asm volatile("movl %%cr2, %0\n" : "=r"(faulting_address));
    return faulting_address;
}

/**
 * Get the active page table
 *
 * @returns the physical base address of the currently active page directory
 */
inline physical_address_t ia32_mmu_t::get_active_pagetable() ALWAYS_INLINE
{
    physical_address_t ret;
    asm volatile ("movl %%cr3, %0\n" : "=a"(ret));
    return ret;
}

/**
 * Sets the active page table
 *
 * @param page_dir_physical page directory physical base address
 */
inline void ia32_mmu_t::set_active_pagetable(physical_address_t page_dir_physical) ALWAYS_INLINE
{
    asm volatile ("movl %0, %%cr3\n" :: "r"(page_dir_physical));
}

// inline void ia32_mmu_t::set_active_pagetable(x86_protection_domain_t& pdom)
// {
//     set_active_pagetable(pdom.physical_page_directory);
// }
