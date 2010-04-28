//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2010, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "bootinfo.h"
#include "new.h"

/*!
 * boot info page layout
 * -------------------- START of page
 * 4 bytes magic (0xbeefdea1)
 * 4 bytes offset of first free byte
 * then a list of bootrec_t subtypes
 * (free space)
 * -------------------- END of page
 */

enum bootrec_type_e
{
    bootrec_module = 1, // loadable module info
    bootrec_memory_map, // memory map info
    bootrec_command_line, // command line info
    bootrec_device_tree,
    end,
    bootrec_multiboot // FIXME: remove me
};

class bootrec_t
{
public:
    uint16_t type;
    uint16_t size;
};

class bootrec_module_t : public bootrec_t
{
public:
    uint32_t start;
    uint32_t end;
    char*    name;
};

union bootrec_info_t
{
    bootrec_t        rec;
    bootrec_module_t module;
};

bootinfo_t::bootinfo_t(bool create_new)
{
    if (create_new)
    {
        magic = BI_MAGIC;
        free = reinterpret_cast<char*>(this) + 8;
    }
}

//! Store module info, memmap and command line from multiboot header.
bool bootinfo_t::append(multiboot_t* mb) // TODO: check remaining space
{
    // append modules
    for (uint32_t i = 0; i < mb->module_count(); i++)
    {
        modinfo_t* mod = mb->module(i);

        bootrec_t* bm = new(free) bootrec_t;
        bm->type = bootrec_module;
        bm->size = 8 + strlen(mod->name) + 1;
        *reinterpret_cast<uint32_t*>(free + 4) = mod->mod_start;
        *reinterpret_cast<uint32_t*>(free + 8) = mod->mod_end;
        memcpy(free + 12, mod->name, strlen(mod->name) + 1);
        free += bm->size;
    }
    // append command line
    // append memory map
    bootrec_t* bm = new(free) bootrec_t;
    bm->type = bootrec_multiboot;
    bm->size = mb->size();
    free += sizeof(bm);
    mb->copy(free);
    free += bm->size;
    return true;
}

// bool bootinfo_t::append_mmap_entry(multiboot_t::mmap_entry_t* entry)
// {
//     const size_t entry_size = sizeof(multiboot_t::mmap_entry_t);
//     if (size() + entry_size > PAGE_SIZE - optional_fields_size())
//         return false;
// 
//     multiboot_t::header_t* header = multiboot_header();
//     uint32_t end = uint32_t(this) + size();
// 
//     memutils::copy_memory(reinterpret_cast<void*>(end), entry, entry_size);
//     reinterpret_cast<multiboot_t::mmap_entry_t*>(end)->set_entry_size(entry_size); // ignore any extra fields
//     header->mmap.set_size(header->mmap.size() + entry_size);
//     increase_size(entry_size);
// 
//     return true;
// }

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
