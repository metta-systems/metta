//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "multiboot.h"

/*!
* Provide access to boot info page structures.
*/
class bootinfo_t
{
public:
    bootinfo_t(address_t bootinfo_page) : boot_info(bootinfo_page) {}
    size_t size();
    multiboot_t::header_t* multiboot_header();
private:
    address_t boot_info;
};

inline size_t bootinfo_t::size()
{
    return *reinterpret_cast<uint32_t*>(boot_info);
}

inline multiboot_t::header_t* bootinfo_t::multiboot_header()
{
    return reinterpret_cast<multiboot_t::header_t*>(boot_info + 4);
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
