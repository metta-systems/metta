//
// Part of Metta OS. Check https://atta-metta.net for latest version.
//
// Copyright 2007 - 2017, Stanislav Karchebnyy <berkus@atta-metta.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Architecture-dependent defines.
// Do not put any functions in this file: it is used with assembler. (FIXME?)
//
#pragma once

#include "types.h"

#define ANY_ADDRESS ((address_t)-1)
#define NO_ADDRESS  ((address_t)-1)
/* definitions for owner field of ramtab */
#define OWNER_NONE    0x0     /* physical frame is unused by anyone        */
#define OWNER_SYSTEM  0x1     /* physical frame is owned by us (mmgmt etc) */

#define NULL_PDID            ((protection_domain_v1::id)~0U)

//======================================================================================================================
// MMU
//======================================================================================================================

// Last 4 megs of address space are for recursive page directory
// @fixme: remove this recursive pd crap.
#define VIRTUAL_PAGE_TABLES     0xffc00000 // page tables addressable from this base
#define VIRTUAL_PAGE_DIRECTORY  0xfffff000 // page directory addressable from this base //0xFFBFF000)
// Next to last 4 megs are for kernel temporary mappings.
#define KERNEL_VIRTUAL_MAPPINGS 0xff800000

static const size_t    PAGE_SIZE = 0x1000;
static const address_t PAGE_MASK = 0xFFFFF000;
static const size_t    PAGE_WIDTH = 12; // replace this with page_t::width
static const size_t    FRAME_WIDTH = 12;

// Page attributes
#define IA32_PAGE_PRESENT        (1<<0)
#define IA32_PAGE_WRITABLE       (1<<1)
#define IA32_PAGE_USER           (1<<2)
#define IA32_PAGE_WRITE_THROUGH  (1<<3)
#define IA32_PAGE_CACHE_DISABLE  (1<<4)
#define IA32_PAGE_ACCESSED       (1<<5)
#define IA32_PAGE_DIRTY          (1<<6)
#define IA32_PAGE_4MB            (1<<7) // only in PDE
#define IA32_PAGE_GLOBAL         (1<<8)
// Custom types
#define IA32_PAGE_SWAPPED        (1<<9)
#define IA32_PAGE_COW            (1<<10)

// CR0 register
#define IA32_CR0_PE (1 <<  0)   /**< enable protected mode                                       */
#define IA32_CR0_WP (1 << 16)   /**< force write protection on user read only pages for kernel   */
#define IA32_CR0_AM (1 << 18)   /**< enable alignment checks                                     */
#define IA32_CR0_PG (1 << 31)   /**< enable paging                                               */

// CR4 register
#define IA32_CR4_VME        (1 << 0)  /**< virtual 8086 mode extensions                  */
#define IA32_CR4_PVI        (1 << 1)  /**< protected mode virtual interrupts             */
#define IA32_CR4_TSD        (1 << 2)  /**< timestamp disable (forbid user rdtsc when set)*/
#define IA32_CR4_DE         (1 << 3)  /**< debugging extensions                          */
#define IA32_CR4_PSE        (1 << 4)  /**< page size extensions (enable 4MB pages)       */
#define IA32_CR4_PAE        (1 << 5)  /**< physical address extension (enable 2MB pages) */
#define IA32_CR4_MCE        (1 << 6)  /**< enable machine check exceptions               */
#define IA32_CR4_PGE        (1 << 7)  /**< enable global pages                           */
#define IA32_CR4_PCE        (1 << 8)  /**< enable rdpmc in user-mode                     */
#define IA32_CR4_OSFXSR     (1 << 9)  /**< enable SSE and FPU fast save/restore          */
#define IA32_CR4_OSXMMEXCPT (1 << 10) /**< enable unmasked SSE exceptions                */
#define IA32_CR4_VMXE       (1 << 13) /**< enable virtual machine extensions             */
#define IA32_CR4_SMXE       (1 << 14) /**< enable safer mode extensions                  */
#define IA32_CR4_PCIDE      (1 << 17) /**< enable process-context identifiers            */
#define IA32_CR4_OSXSAVE    (1 << 18) /**< enable xsave and cpu extended states          */
#define IA32_CR4_SMEP       (1 << 20) /**< enable supervisor mode execution protection   */
#define IA32_CR4_SMAP       (1 << 21) /**< enable supervisor mode access protection      */

// Machine-specific registers
#define X86_MSR_PMCTR0  0xc1
#define X86_MSR_PMCTR1  0xc2
#define X86_MSR_EVSEL0  0x186
#define X86_MSR_EVSEL1  0x187
