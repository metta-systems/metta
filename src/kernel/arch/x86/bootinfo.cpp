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

//======================================================================================================================
// internal structures
//======================================================================================================================

/*!
 * boot info page layout
 * -------------------- START of page
 * 4 bytes magic (0xbeefdea1)
 * 4 bytes offset of first free byte
 * then a list of bootrec_t subtypes
 * (free space)
 * -------------------- END of page
 *
 * bootrec subtypes:
 * size is the size of the entire record, including bootrec_t header (so, cur + size will equal start of next record).
 *
 */

enum bootrec_tag_e
{
    bootrec_module = 1,   // loadable module info
    bootrec_memory_map,   // memory map info
    bootrec_command_line, // command line info
    bootrec_device_tree,
    end
};

class bootrec_t
{
public:
    uint16_t tag;
    uint16_t size;
};

class bootrec_module_t : public bootrec_t
{
public:
    uint32_t number; // original module number
    uint64_t start;
    uint64_t end;
    char*    name;
};

// Each memmap entry is separate.
class bootrec_mmap_entry_t : public bootrec_t
{
public:
    uint64_t start;
    uint64_t length;
    uint32_t type;
};

class bootrec_cmdline_t : public bootrec_t
{
public:
    char* cmdline;
};

union bootrec_info_t
{
    bootrec_t*            rec;
    bootrec_module_t*     module;
    bootrec_mmap_entry_t* memmap;
    bootrec_cmdline_t*    cmdline;
    char*                 generic;
};

//======================================================================================================================
// mmap_iterator
//======================================================================================================================

bootinfo_t::mmap_iterator::mmap_iterator(void* entry, void* end)
    : end(end)
{
    set(entry);
}

void bootinfo_t::mmap_iterator::set(void* entry)
{
    ptr = entry;
    if (ptr)
    {
        bootrec_mmap_entry_t* e = reinterpret_cast<bootrec_mmap_entry_t*>(entry);
        start = e->start;
        size = e->length;
        type = e->type;
    }
}

multiboot_t::mmap_entry_t bootinfo_t::mmap_iterator::operator *()
{
    multiboot_t::mmap_entry_t entry;
    entry.set_region(start, size, (multiboot_t::mmap_entry_t::entry_type_e)type);
    return entry;
}

void bootinfo_t::mmap_iterator::operator ++()
{
    bootrec_info_t info;
    info.generic = reinterpret_cast<char*>(ptr);
    info.generic += info.rec->size; // move ahead one entry
    while (info.generic < end)
    {
        if (info.rec->tag == bootrec_memory_map)
        {
            set(info.generic);
            return;
        }
        info.generic += info.rec->size;
    }
    set(0);
    end = 0; // iterator has exhausted!
    return;
}

//======================================================================================================================
// module_iterator
//======================================================================================================================

// TODO: IMPLEMENT!

//======================================================================================================================
// bootinfo_t
//======================================================================================================================

bootinfo_t::bootinfo_t(bool create_new)
{
    if (create_new)
    {
        magic = BI_MAGIC;
        free = reinterpret_cast<char*>(this) + sizeof(magic) + sizeof(free);
    }
}

bool bootinfo_t::get_module(uint32_t number, address_t& start, address_t& end, const char*& name)
{
    bootrec_info_t info;
    info.generic = reinterpret_cast<char*>(this + 1);
    while (info.generic < free)
    {
        if (info.rec->tag == bootrec_module)
        {
            if (info.module->number == number)
            {
                start = info.module->start;
                end = info.module->end;
                name = info.module->name;
                return true;
            }
        }
        info.generic += info.rec->size;
    }
    return false;
}

bool bootinfo_t::get_cmdline(const char*& cmdline)
{
    bootrec_info_t info;
    info.generic = reinterpret_cast<char*>(this + 1);
    while (info.generic < free)
    {
        if (info.rec->tag == bootrec_command_line)
        {
            cmdline = info.cmdline->cmdline;
            return true;
        }
        info.generic += info.rec->size;
    }
    return false;
}

bootinfo_t::mmap_iterator bootinfo_t::mmap_begin()
{
    bootrec_info_t info;
    info.generic = reinterpret_cast<char*>(this + 1);
    while (info.generic < free)
    {
        if (info.rec->tag == bootrec_memory_map)
        {
            return mmap_iterator(info.memmap, free);
        }
        info.generic += info.rec->size;
    }
    return mmap_end();
}

bootinfo_t::mmap_iterator bootinfo_t::mmap_end()
{
    return mmap_iterator(0, 0);
}

// TODO: check remaining space
bool bootinfo_t::append_module(uint32_t number, multiboot_t::modinfo_t* mod)
{
    if (!mod)
        return false;

    bootrec_module_t* bootmod = new(free) bootrec_module_t;
    bootmod->tag = bootrec_module;
    bootmod->size = sizeof(bootrec_module_t) + memutils::string_length(mod->str) + 1;

    bootmod->number = number;
    bootmod->start = mod->mod_start;
    bootmod->end = mod->mod_end;
    char* name = free + sizeof(bootrec_module_t);
    bootmod->name = name;

    memutils::copy_string(name, mod->str);

    free += bootmod->size;
    return true;
}

// TODO: check remaining space
bool bootinfo_t::append_mmap(multiboot_t::mmap_entry_t* entry)
{
    if (!entry)
        return false;

    bootrec_mmap_entry_t* bootmmap = new(free) bootrec_mmap_entry_t;
    bootmmap->tag = bootrec_memory_map;
    bootmmap->size = sizeof(bootrec_mmap_entry_t);

    bootmmap->start = entry->address();
    bootmmap->length = entry->size();
    bootmmap->type = entry->type();

    free += bootmmap->size;
    return true;
}

// TODO: check remaining space
bool bootinfo_t::append_cmdline(const char* cmdline)
{
    if (!cmdline)
        return false;

    bootrec_cmdline_t* bootcmd = new(free) bootrec_cmdline_t;
    bootcmd->tag = bootrec_command_line;
    bootcmd->size = sizeof(bootrec_cmdline_t) + memutils::string_length(cmdline) + 1;

    char* name = free + sizeof(bootrec_cmdline_t);
    bootcmd->cmdline = name;

    memutils::copy_string(name, cmdline);

    free += bootcmd->size;
    return true;
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
