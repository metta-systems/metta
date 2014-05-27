//
// Part of Metta OS. Check http://atta-metta.net for latest version.
//
// Copyright 2007 - 2012, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include <new>
#include "multiboot.h"
#include "memory.h"
#include "memutils.h"
#include "iterator"
#include "macros.h"
#include "memory_v1_interface.h"

class module_loader_t;

// TODO: We need to abstract frames module from the format of bootinfo page,
// so we add a type for memory_map and make it hide the fact that it uses the bootinfo_page
// we pass the memory_map type to frames_mod.

/// use bootinfo_t::mmap_begin/end for now, but probably bootinfo_t should return memory_map_t in request for memmap?

/*class memory_map_t
{
public:
    memory_map_t();
    memory_map_t(bootinfo_t* bi); // this hides bootinfo behind mmap type

    // memory item returned by the iterator
    class entry_t
    {
    public:
        bool is_free();
        physical_address_t start();
        size_t size();
    };

    class memory_map_iterator_t : public std::iterator<std::forward_iterator_tag, memory_map_t::memory_map_entry_t>
    {
    public:
        memory_map_iterator_t();
        memory_map_t::entry_t operator *();
        void operator ++();
        void operator ++(int);
        inline bool operator == (const memory_map_iterator_t& other) { return ptr == other.ptr; }
        inline bool operator != (const memory_map_iterator_t& other) { return ptr != other.ptr; }
    };

    typedef memory_map_iterator_t iterator;
    iterator begin(); // see bootinfo_t::mmap_begin()
    iterator begin() const;
    iterator end();
    iterator end() const;
    iterator rbegin();
    iterator rbegin() const;
    iterator rend();
    iterator rend() const;
};*/

/**
 * @class bootinfo_t
 * Provides access to boot info page structures.
 *
 * Common way of accessing it is to create an instance of bootinfo_t using placement new at the location
 * of bootinfo_t::ADDRESS, e.g.:
 * <code>
 * bootinfo_t* bi = new(bootinfo_t::ADDRESS) bootinfo_t;
 * </code>
 * Then you can add items or query items.
 * Boot info page is limited to a native page size (4Kb by default) to simplify memory allocation.
 */
class bootinfo_t
{
    uint32_t  magic;
    char*     free;
    address_t first_module_address;
    address_t last_available_module_address;

    static const uint32_t BI_MAGIC = 0xbeefdea1;
    multiboot_t::mmap_entry_t* find_matching_entry(address_t start, size_t size, int& n_way);

public:
    /** Rather arbitrary location for the bootinfo page. */
    static void* ADDRESS; // not an enum because of placement new()

    /** Iterator for going over available physical memory map entries. */
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
        multiboot_t::mmap_entry_t* operator *(); // allows in-place modification of mmap entries
        void operator ++();
        void operator ++(int);
        inline bool operator == (const mmap_iterator& other) { return ptr == other.ptr; }
        inline bool operator != (const mmap_iterator& other) { return ptr != other.ptr; }
    };

    /** Iterator for going over available virtual memory mapping entries. */
    class vmap_iterator : public std::iterator<std::forward_iterator_tag, memory_v1::mapping>
    {
//        address_t start;
//        size_t size;
//        int type;
        void* ptr;
        void* end;

        void set(void* entry);

    public:
        vmap_iterator() : ptr(0), end(0) {}
        vmap_iterator(void* entry, void* end);
        memory_v1::mapping* operator *(); // we don't need to in-place modify memory mappings, but lets keep it this way for simplicity at the moment.
        void operator ++();
        void operator ++(int);
        inline bool operator == (const vmap_iterator& other) { return ptr == other.ptr; }
        inline bool operator != (const vmap_iterator& other) { return ptr != other.ptr; }
    };

    struct module_entry
    {
        uint64_t start, end;
        const char* name;
    };

public:
    bootinfo_t(bool create_new = false);
    inline bool is_valid() const { return (magic == BI_MAGIC) and (size() <= PAGE_SIZE); }
    inline size_t size() const { return reinterpret_cast<const char*>(free) - reinterpret_cast<const char*>(this); }

    inline bool will_overflow(size_t add_size)
    {
        return (size() + add_size) > PAGE_SIZE; //((free & PAGE_MASK) != ((free + add_size) & PAGE_MASK));
    }

    /**
     * NB! Module loader received from this bootinfo will modify it,
     * so do not try to use two modules loaders from two different bootinfos at once!
     * (Don't use more than one bootinfo at a time at all, they are not concurrency-safe!)
     */
    module_loader_t modules();

    /** Where the last loaded module ends. */
    address_t module_load_end() const { return last_available_module_address; }

    /** Load module ELF file by number. */
    bool get_module(uint32_t number, address_t& start, address_t& end, const char*& name);
    // Load module by name.
//     bool get_module(const char* name, module_info_t& mod);
    bool get_cmdline(const char*& cmdline);

    mmap_iterator mmap_begin();
    mmap_iterator mmap_end();

    vmap_iterator vmap_begin();
    vmap_iterator vmap_end();

    /** Append parts of multiboot header in a format suitable for bootinfo page. */
    bool append_module(uint32_t number, multiboot_t::modinfo_t* mod);
    /** Append modules loaded from the multiboot modules (initrd etc) */
    bool append_module(const char* name, multiboot_t::modinfo_t* mod);

    bool append_mmap(multiboot_t::mmap_entry_t* entry);
    bool append_vmap(address_t vstart, address_t pstart, size_t size);
    bool append_cmdline(const char* cmdline);

    address_t find_usable_physical_memory_top();
    address_t find_highmem_range_of_at_least(size_t bytes);

    bool use_memory(address_t start, size_t size, multiboot_t::mmap_entry_t::entry_type_e type);

    inline bool use_memory(void* start, size_t size, multiboot_t::mmap_entry_t::entry_type_e type)
    {
        return use_memory(reinterpret_cast<address_t>(start), size, type);
    }

    // Debug aids.
    void print_memory_map();
};
