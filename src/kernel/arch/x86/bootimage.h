//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2012, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "types.h"
#include "cstring.h"
#include "module_namespace.h"

namespace bootimage_n { struct namespace_entry_t; }

/**
 * Bootimage is similar to Nemesis' nexus - it contains information about modules, dependencies, namespaces
 * and everything else required for successful startup.
 *
 * Index information in bootimage should allow quick and easy dependency calculation and module instantiation.
 * Modules can be ELF executables or data blobs loaded and mapped at specified address in memory.
 *
 * Deps are lists of items from common stringtable. (ofs,len) pairs for ndeps count.
 *
 * Root entry in bootimage is main startup code, called "root domain".
 */
class bootimage_t
{
public:
    class modinfo_t
    {
    public:
        modinfo_t(address_t st, size_t sz) : start(st), size(sz) {}

        address_t start;
        size_t    size;
    };

    class namespace_t
    {
        address_t base;
        uint32_t n_entries;
        bootimage_n::namespace_entry_t* entries;
    public:
        namespace_t() {}
        namespace_t(address_t b, void* ptr) { set(b, ptr); }
        void set(address_t base, void* loc);

        // find an entry in the namespace with key key and return it's int value
        bool get_int(cstring_t key, int& value);
        bool get_string(cstring_t key, cstring_t& value);
        bool get_symbol(cstring_t key, void*& value);

        void dump_all_keys();
    };

    bootimage_t(const char* name, address_t start, address_t end);

    modinfo_t find_root_domain(namespace_t* namesp);
    modinfo_t find_module(const char* name);
    // namespace_t find_namespace(const char* name); // find namespace for module named name

    bool valid();

private:
    address_t location;
    address_t end;
};

