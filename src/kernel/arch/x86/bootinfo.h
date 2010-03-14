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

class bootrec_t
{
    uint32_t type;
    uint32_t version;
    uint32_t size;
};

class bootrec_module_t : public bootrec_t
{
};

class bootrec_efi_t : public bootrec_t
{
};

// put whole multiboot header into the bootinfo page
class bootrec_multiboot_t : public bootrec_t
{
};

class bootrec_device_tree_t : public bootrec_t
{
};

/*!
 * Provides access to boot info page structures.
 *
 * boot info page layout
 * -------------------- START of page
 * 4 bytes magic
 * 4 bytes version
 * 4 bytes size
 * 4 bytes first entry offset
 * 4 bytes entry count
 * then a list of bootrec_t subtypes
 * (free space)
 * -------------------- END of page
 */
class bootinfo_t
{
    uint32_t magic;
    uint32_t version;
    uint32_t size;
    uint32_t first_entry;
    uint32_t n_entries;

public:
    bootinfo_t(bool existing = false);
    bool valid() const { return magic == BI_MAGIC && version == BI_VERSION; }
    inline size_t size() const { return this->size; }
    bootrec_t* first_entry() const;
    uint32_t num_entries() const { return n_entries; }

    bool append(bootrec_t* rec);

//     multiboot_t::header_t* multiboot_header();
//     bool append_mmap_entry(multiboot_t::mmap_entry_t* entry);
//     void mmap_prepare(multiboot_t::mmap_t* mmap);
};

// inline multiboot_t::header_t* bootinfo_t::multiboot_header()
// {
//     return reinterpret_cast<multiboot_t::header_t*>(this + sizeof(size_t) + sizeof(uint32_t));
// }

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
