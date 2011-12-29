//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2011, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "types.h"
#include "module_namespace.h"

/*!
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

    bootimage_t(const char* name, address_t start, address_t end);

    modinfo_t find_root_domain(module_namespace_t* namesp);
    modinfo_t find_module(const char* name);
    modinfo_t find_namespace(const char* name);

    bool valid();

private:
    address_t location;
    address_t end;
};
 
// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
