//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2012, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Loader with multiboot specification support.
//
#include "multiboot.h"
#include "bootinfo.h"
#include "elf_parser.h"
#include "new"
#include "default_console.h"
#include "debugger.h"
#include "module_loader.h"
#include "bootimage.h"

/**
 * Check if a valid multiboot info structure is present.
 */
bool mbi_probe()
{
    kconsole << "mbi_probe()" << endl;
    multiboot_t* _mbi = multiboot_t::prepare();

    if (_mbi == NULL)
        return false;

    // Make a safe copy of the MBI structure itself.
    bootinfo_t* bi = new(bootinfo_t::ADDRESS) bootinfo_t(true);

    // We need info about memory map, modules and command line.
    multiboot_t::mmap_t* memmap = _mbi->memory_map();
    if (memmap)
    {
        multiboot_t::mmap_entry_t* mmi = memmap->first_entry();
        while (mmi)
        {
            bi->append_mmap(mmi);
            mmi = memmap->next_entry(mmi);
        }
    }

    // mark loaded modules memory as used in the bootinfo (so we don't overwrite them later on)
    for (size_t i = 0; i < _mbi->module_count(); i++)
    {
        auto mod = _mbi->module(i);
        bi->append_module(i, mod);
        if (!bi->use_memory(mod->mod_start, mod->mod_end - mod->mod_start, multiboot_t::mmap_entry_t::loader_reclaimable))
        {
            kconsole << __FUNCTION__ << ": unable to reserve module memory in memmap!" << endl;
        }
    }

    bi->append_cmdline(_mbi->cmdline());

    return true;
}


//*****************************************************************************************************************
// TODO:
// record updated location info about relocated modules directly into bootinfo page entries.
// this will copy only needed code and data (e.g. booting without "debug" will not copy debug infos)
//*****************************************************************************************************************

extern "C" void arch_prepare();

/**
 * Init function that understands multiboot info structure.
 *
 * The procedure goes as follows:
 * - We have mbi inside the bootinfo page already.
 * - ELF-load the proper nucleus module.
 * - initialize it and mark memory as used.
 * - ELF-load the root-domain bootstrapper.
 * - return entry point of root-domain kick-off sequence. root-domain will run in ring3.
 * @return entry point for the kernel.
 */
address_t mbi_prepare()
{
    multiboot_t* mbi = multiboot_t::prepare();

    // Run architecture-dependent part of the bootstrap, used to be kernel_startup() in startup.cpp
    arch_prepare();

    bootinfo_t* bi = new(bootinfo_t::ADDRESS) bootinfo_t;

    address_t start, end;
    const char* name;
    if (!bi->get_module(0, start, end, name))
    {
        PANIC("Bootimage not found!");
    }

    bootimage_t bootimage(name, start, end);

    // Load and relocate nucleus.
    elf_parser_t elf(bootimage.find_module("nucleus").start);
    if (!elf.is_valid())
        PANIC("Invalid nucleus ELF image!");
    void (*nucleus_init)() = reinterpret_cast<void (*)()>(bi->modules().load_module("nucleus", elf, "nucleus_init"));
    nucleus_init();

    // Load and relocate root domain bootstrapper.
    bootimage_t::modinfo_t mi = bootimage.find_root_domain(0);
    kconsole << "Root domain at " << (unsigned)mi.start << ", size " << int(mi.size) << " bytes." << endl;

    elf_parser_t elf2(mi.start);
    if (!elf2.is_valid())
        PANIC("Invalid root_domain ELF image!");

    // Return bootstrapper's entry point address.
    return (address_t)bi->modules().load_module("root_domain", elf2, "module_entry");
}
