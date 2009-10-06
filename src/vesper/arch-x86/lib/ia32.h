//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Architecture-dependent defines.
// Do not put any functions in this file: it is used with assembler. (FIXME?)
//
#pragma once

//======================================================================================================================
// MMU
//======================================================================================================================

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
#define IA32_PAGE_SUPER          (1<<7)
#define IA32_PAGE_GLOBAL         (1<<8)

// CR0 register
#define IA32_CR0_PE (1 <<  0)   /* enable protected mode    */
#define IA32_CR0_WP (1 << 16)   /* force write protection on user read only pages for kernel   */
#define IA32_CR0_PG (1 << 31)   /* enable paging        */

// CR4 register
#define IA32_CR4_PSE    (1 <<  4)   /* page size extension (4MB)    */
#define IA32_CR4_PGE    (1 <<  7)   /* enable global pages      */

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
