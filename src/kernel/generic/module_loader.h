//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2011, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "elf_parser.h"

class bootinfo_t;

/*!
 * Loads components from initramfs to predefined memory area and relocates them.
 * Constructor receives addresses of last_available_address field from
 * inside bootinfo page.
 * @internal This class shall be used only internally during bootup.
 */
class module_loader_t
{
public:
    module_loader_t(bootinfo_t* parent, address_t* first_used_address, address_t* last_available_address)
        : d_parent(parent)
        , d_first_used_address(first_used_address)
        , d_last_available_address(last_available_address)
    {}

    /*!
     * @returns pointer to given closure.
     */
    void* load_module(const char* name, elf_parser_t& module, const char* closure_name);

private:
    bootinfo_t* d_parent;
    address_t*  d_first_used_address;
    address_t*  d_last_available_address;
};

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
