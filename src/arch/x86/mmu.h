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
#include "x86_protection_domain.h"

class ia32_mmu_t
{
public:
    static void flush_page_directory(bool global = false);
    static void flush_page_directory_entry(address_t addr);
    static void enable_super_pages();
    static void enable_global_pages();
    static void enable_paged_mode();
    static address_t get_pagefault_address(void);
    static physical_address_t get_active_pagetable(void);
    static void set_active_pagetable(physical_address_t page_dir_physical);
    static void set_active_pagetable(x86_protection_domain_t& pdom);
};

/*!
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

/*!
 * Flushes the TLB entry for a linear address
 *
 * @param virt linear address
 */
inline void ia32_mmu_t::flush_page_directory_entry(address_t virt)
{
    asm volatile ("invlpg (%0)\n" :: "r"(virt));
}

// FIXME: ia32_cpu_t::cr4_enable()?
inline void ia32_cr4_set(uint32_t flag)
{
    uint32_t dummy;
    asm volatile ("movl %%cr4, %0\n"
                  "orl %1, %0\n"
                  "movl %0, %%cr4\n"
                  : "=r"(dummy)
                  : "i"(flag));
}

/*!
 * Enables extended page size (4M) support for IA32
 */
inline void ia32_mmu_t::enable_super_pages()
{
    ia32_cr4_set(IA32_CR4_PSE);
}

/*!
 * Enables global page support for IA32
 */
inline void ia32_mmu_t::enable_global_pages()
{
    ia32_cr4_set(IA32_CR4_PGE);
}

/*!
 * Enables paged mode for IA32
 */
inline void ia32_mmu_t::enable_paged_mode()
{
    asm volatile ("mov %0, %%cr0\n" :: "r"(IA32_CR0_PG | IA32_CR0_WP | IA32_CR0_PE));
}

/*!
 * @returns the linear address of the last pagefault
 */
inline address_t ia32_mmu_t::get_pagefault_address()
{
    uint32_t faulting_address;
    asm volatile("movl %%cr2, %0\n" : "=r"(faulting_address));
    return faulting_address;
}

/*!
 * Get the active page table
 *
 * @returns the physical base address of the currently active page directory
 */
inline physical_address_t ia32_mmu_t::get_active_pagetable()
{
    physical_address_t ret;
    asm volatile ("movl %%cr3, %0\n" : "=a"(ret));
    return ret;
}

/*!
 * Sets the active page table
 *
 * @param page_dir_physical page directory physical base address
 */
inline void ia32_mmu_t::set_active_pagetable(physical_address_t page_dir_physical)
{
    asm volatile ("movl %0, %%cr3\n" :: "r"(page_dir_physical));
}

inline void ia32_mmu_t::set_active_pagetable(x86_protection_domain_t& pdom)
{
    set_active_pagetable(pdom.physical_page_directory);
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
