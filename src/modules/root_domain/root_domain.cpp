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
    kconsole << "Root domain at " << (unsigned)mi.start << ", size " << mi.size << " bytes." << endl;

    elf.parse(mi.start);
    if (!elf.is_valid())
        PANIC("Invalid root_domain ELF image!");

    bootinfo_t* bi = new(BOOTINFO_PAGE) bootinfo_t;
    entry_point = (address_t)bi->get_module_loader().load_module("root_domain", elf, NULL);
}

address_t root_domain_t::entry()
{
    return entry_point;
}
