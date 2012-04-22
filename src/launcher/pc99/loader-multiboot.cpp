//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2011, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Loader version that supports multiboot specification.
//
#include "multiboot.h"
#include "bootinfo.h"
#include "elf_parser.h"
#include "new.h"
#include "default_console.h"
#include "debugger.h"
#include "module_loader.h"
#include "bootimage.h"
#include "root_domain.h"

/*!
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

    for (size_t i = 0; i < _mbi->module_count(); i++)
    {
        bi->append_module(i, _mbi->module(i));
    }

    // mark loaded modules memory as used in the bootinfo (so we don't overwrite them later on)
    for (size_t i = 0; i < _mbi->module_count(); i++)
    {
        if (!bi->use_memory(_mbi->module(i)->mod_start, _mbi->module(i)->mod_end - _mbi->module(i)->mod_start))
        {
            kconsole << __FUNCTION__ << ": unable to reserve module memory in memmap!" << endl;
            break;
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

/*!
 * Init function that understands multiboot info structure.
 *
 * The procedure goes as follows:
 * - We have mbi inside the bootinfo page already.
 * - ELF-load the proper nucleus module.
 * - initialize it and mark memory as used.
 * - ELF-load the root-domain bootstrapper.
 * - return entry point of root-domain kick-off sequence. -- TODO, this means root_domain starts in ring0, we better launch it in ring3 directly. - launch_kernel() may switch to ring3
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
    if (!bi->get_module(1, start, end, name))
    {
        PANIC("Bootimage not found!");
    }

    bootimage_t bootimage(name, start, end);
    // bootimage_t bootimage(mbi->module(0)->mod_start);

    // Load and relocate nucleus.
    elf_parser_t elf(bootimage.find_module("nucleus").start);
    void (*nucleus_init)();
    nucleus_init = (void (*)())bi->get_module_loader().load_module("nucleus", elf, "nucleus_init");
    nucleus_init();

    // Load and relocate root domain bootstrapper.
    root_domain_t root_dom(bootimage);
//     kconsole << "+ root_domain entry @ 0x" << root_dom.entry() << endl;

    // Return bootstrapper's entry point address.
    // return (address_t)bi->get_module_loader().load_module("root_domain", elf, NULL);
    return root_dom.entry();
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
