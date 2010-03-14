//
// Architecture-dependent defines.
// Do not put any functions in this file: it is used with assembler. (FIXME?)
//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2010, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "types.h"

//======================================================================================================================
// MMU
//======================================================================================================================

// Last 4 megs of address space are for recursive page directory
#define VIRTUAL_PAGE_TABLES     0xffc00000 // page tables addressable from this base
#define VIRTUAL_PAGE_DIRECTORY  0xfffff000 // page directory addressable from this base //0xFFBFF000)
// Next to last 4 megs are for kernel temporary mappings.
#define KERNEL_VIRTUAL_MAPPINGS 0xff800000

const size_t    PAGE_SIZE = 0x1000;
const address_t PAGE_MASK = 0xFFFFF000;

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
#define IA32_CR0_PE (1 <<  0)   /*!< enable protected mode    */
#define IA32_CR0_WP (1 << 16)   /*!< force write protection on user read only pages for kernel   */
#define IA32_CR0_PG (1 << 31)   /*!< enable paging        */

// CR4 register
#define IA32_CR4_PSE    (1 << 4)   /*!< page size extensions (enable 4MB pages)       */
#define IA32_CR4_PAE    (1 << 5)   /*!< physical address extension (enable 2MB pages) */
#define IA32_CR4_PGE    (1 << 7)   /*!< enable global pages                           */

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
