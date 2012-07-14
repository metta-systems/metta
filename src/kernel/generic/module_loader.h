//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2012, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "elf_parser.h"
#include "heap_allocator.h"
#include <unordered_map>
#include <vector>

class bootinfo_t;

class module_symbols_t
{
public:
    typedef const char* key_type;
    typedef elf32::symbol_t* value_type;
    typedef std::heap_allocator<std::pair<key_type, value_type>> symmap_alloc;
    typedef std::unordered_map<key_type, value_type, std::hash<key_type>, std::equal_to<key_type>, symmap_alloc> symmap;

    module_symbols_t(symmap&& s) : symtab(s) {} // move ctor

    inline symmap& all_symbols() { return symtab; }
    symmap starting_with(const char* prefix);
    symmap ending_with(const char* suffix);

private:
    symmap symtab;
};

/**
 * Loads components from initramfs to predefined memory area and relocates them.
 * Constructor receives addresses of last_available_address field from
 * inside bootinfo page.
 * @internal This class shall be used only internally during bootup.
 */
class module_loader_t
{
public:
    typedef std::heap_allocator<const char*> strvec_alloc;
    typedef std::vector<const char*, strvec_alloc> strvec;

    module_loader_t(bootinfo_t* parent, address_t* first_used_address, address_t* last_available_address)
        : d_parent(parent)
        , d_first_used_address(first_used_address)
        , d_last_available_address(last_available_address)
    {}

    /**
     * @returns pointer to given closure.
     */
    void* load_module(const char* name, elf_parser_t& module, const char* closure_name);

    module_symbols_t symtab_for(const char* name, const char* suffix);
    strvec loaded_module_names();

private:
    bootinfo_t* d_parent;
    address_t*  d_first_used_address;
    address_t*  d_last_available_address;
};
