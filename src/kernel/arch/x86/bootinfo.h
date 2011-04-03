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
#include "iterator"
#include "module_loader.h"
#include "macros.h"
#include "new.h"

// Rather arbitrary location for the bootinfo page.
static void* BOOTINFO_PAGE UNUSED_ARG = (void*)0x8000;

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
    uint32_t  magic;
    char*     free;
    address_t last_available_module_address;

    static const uint32_t BI_MAGIC = 0xbeefdea1;

public:
    /* Iterator for going over available memory map entries. */
    class mmap_iterator : public std::iterator<std::forward_iterator_tag, multiboot_t::mmap_entry_t>
    {
        address_t start;
        size_t size;
        int type;
        void* ptr;
        void* end;

        void set(void* entry);

    public:
        mmap_iterator() : ptr(0), end(0) {}
        mmap_iterator(void* entry, void* end);
        multiboot_t::mmap_entry_t operator *();
        void operator ++();
        void operator ++(int);
        inline bool operator == (const mmap_iterator& other) { return ptr == other.ptr; }
        inline bool operator != (const mmap_iterator& other) { return ptr != other.ptr; }
    };

    struct module_entry
    {
        uint64_t start, end;
        const char* name;
    };

    /* Iterator for going over available modules. */
    class module_iterator : public std::iterator<std::forward_iterator_tag, module_entry>
    {
        uint64_t mod_start;
        uint64_t mod_end;
        const char* name;
        void* ptr;
        void* end;

        void set(void* entry);

    public:
        module_iterator(void* entry, void* end);
        module_entry operator *();
        void operator ++();
        inline bool operator != (const module_iterator& other) { return ptr != other.ptr; }
    };

public:
    bootinfo_t(bool create_new = false);
    inline bool is_valid() const { return magic == BI_MAGIC && size() <= PAGE_SIZE; }
    inline size_t size() const { return reinterpret_cast<const char*>(free) - reinterpret_cast<const char*>(this); }

    inline bool will_overflow(size_t add_size)
    {
        return size() + add_size > PAGE_SIZE; //((free & PAGE_MASK) != ((free + add_size) & PAGE_MASK));
    }

    // NB! Module loader received from this bootinfo will modify it, so do not try to use two modules loaders from
    // two different bootinfos at once! (Don't use more than one bootinfo at a time at all, they are not concurrency-safe!)
    module_loader_t get_module_loader();

    // Load module ELF file by number.
    bool get_module(uint32_t number, address_t& start, address_t& end, const char*& name);
    // Load module by name.
//     bool get_module(const char* name, module_info_t& mod);
    bool get_cmdline(const char*& cmdline);

    mmap_iterator mmap_begin();
    mmap_iterator mmap_end();

    module_iterator module_begin();
    module_iterator module_end();

    // Append parts of multiboot header in a format suitable for bootinfo page.
    bool append_module(uint32_t number, multiboot_t::modinfo_t* mod);
    // Appends modules loaded from the multiboot modules (initrd etc)
    bool append_module(const char* name, multiboot_t::modinfo_t* mod);

    bool append_mmap(multiboot_t::mmap_entry_t* entry);
    bool append_cmdline(const char* cmdline);
};

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
