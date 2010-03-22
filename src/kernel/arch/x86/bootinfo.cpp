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
 * 4 bytes magic
 * 4 bytes address of first free byte
 * then a list of bootrec_t subtypes
 * (free space)
 * -------------------- END of page
 */

enum bootrec_type_e
{
    bootrec_multiboot = 1,
    bootrec_device_tree,
    bootrec_command_line,
    end
};

class bootrec_t
{
public:
    uint16_t type;
    uint16_t size;
};

bootinfo_t::bootinfo_t(bool create_new)
{
    if (create_new)
    {
        magic = BI_MAGIC;
        free = reinterpret_cast<char*>(this) + 8;
    }
}

bool bootinfo_t::append(multiboot_t* mb) // TODO: check remaining space
{
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
