//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "global_descriptor_table.h"

extern "C" void activate_gdt(address_t base); // in activate.s

gdt_entry gdt_entries[5];

/* Singleton instead? */
void global_descriptor_table::init()
{
    static global_descriptor_table gdt;
}

global_descriptor_table::global_descriptor_table()
{
    limit = sizeof(gdt_entries)-1;
    base = (address_t)&gdt_entries;

    gdt_entries[0].set_null();                                // null
    gdt_entries[1].set_seg(0, 0xFFFFFFFF, gdt_entry::code, 0); // ring0 CS
    gdt_entries[2].set_seg(0, 0xFFFFFFFF, gdt_entry::data, 0); // ring0 DS
    gdt_entries[3].set_seg(0, 0xFFFFFFFF, gdt_entry::code, 3); // ring3 CS
    gdt_entries[4].set_seg(0, 0xFFFFFFFF, gdt_entry::data, 3); // ring3 DS

    activate_gdt((address_t)this);
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
