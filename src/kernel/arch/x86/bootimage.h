//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2010, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "types.h"

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
 *
 */
class bootimage_t
{
public:
    bootimage_t(const char* name, address_t start, address_t end);

    address_t find_root_domain(size_t* size);
    address_t find_module(size_t* size, const char* name);

    bool valid();

private:
    address_t location;
    address_t end;
};
 
// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
