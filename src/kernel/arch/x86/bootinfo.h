//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2010, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "multiboot.h"
#include "memory.h"
#include "memutils.h"

// Rather arbitrary location for the bootinfo page.
#define BOOTINFO_PAGE ((void*)0x8000)

/*!
 * Provides access to boot info page structures.
 *
 * Common way of accessing it is to create an instance of bootinfo_t using placement new at the location
 * of BOOTINFO_PAGE, e.g.:
 * bootinfo_t* bi = new(BOOTINFO_PAGE) bootinfo_t;
 * Then you can add items or query items.
 */
class bootinfo_t
{
    uint32_t magic;
    char*    free;

    static const uint32_t BI_MAGIC = 0xbeefdea1;

public:
    bootinfo_t(bool create_new = false);
    inline bool is_valid() const { return magic == BI_MAGIC && size() <= PAGE_SIZE; }
    inline size_t size() const { return reinterpret_cast<const char*>(free) - reinterpret_cast<const char*>(this); }

    // Append parts of multiboot header in a format suitable for bootinfo page.
    bool append_module(uint32_t number, multiboot_t::modinfo_t* mod);
    bool append_mmap(multiboot_t::mmap_entry_t* entry);
    bool append_cmdline(const char* cmdline);
};

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :