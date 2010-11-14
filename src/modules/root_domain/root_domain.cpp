//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2010, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "bootimage.h"
#include "bootinfo.h"
#include "root_domain.h"
#include "panic.h"
#include "default_console.h"
#include "debugger.h"
#include "module_loader.h"

root_domain_t::root_domain_t(bootimage_t& img)
    : ns(0, 0)
{
    bootimage_t::modinfo_t mi = img.find_root_domain(&ns);
    kconsole << "Root domain @ " << (unsigned)mi.start << ", size " << mi.size << " bytes." << endl;
    elf.parse(mi.start);
    if (!elf.is_valid())
        PANIC("invalid root_domain elf image");

    bootinfo_t* bi = new(BOOTINFO_PAGE) bootinfo_t;
    bi->get_module_loader().load_module("root_domain", elf, "entry");///******************************************
    mi.start = 16*MiB;

    elf32::section_header_t* text = elf.section_header(".text");
    address_t entry = elf.get_entry_point();
    ptrdiff_t offset = mi.start - text->addr + text->offset;
    if (!elf.is_relocatable() && offset != 0)
        PANIC("non-relocatable root domain");
    elf.relocate_to(mi.start, 0);
    kconsole << "Root domain relocated by " << (unsigned)offset << endl;
    entry_point = entry + offset;

    bochs_magic_trap();
}

address_t root_domain_t::entry()
{
    return entry_point;
}
