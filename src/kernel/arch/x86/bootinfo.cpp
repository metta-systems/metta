//
// Part of Metta OS. Check http://atta-metta.net for latest version.
//
// Copyright 2007 - 2012, Stanislav Karchebnyy <berkus@atta-metta.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "bootinfo.h"
#include "module_loader.h"

//======================================================================================================================
// internal structures
//======================================================================================================================

void* bootinfo_t::ADDRESS = (void*)0x8000;

//======================================================================================================================
// bootinfo_t
//======================================================================================================================

bootinfo_t::bootinfo_t(bool create_new)
{
    if (create_new)
    {
        magic = BI_MAGIC;
        free = reinterpret_cast<char*>(this + 1);
        first_module_address = 0;
        last_available_module_address = 0;
    }
}

module_loader_t bootinfo_t::modules()
{
    return module_loader_t(this, &first_module_address, &last_available_module_address);
}
