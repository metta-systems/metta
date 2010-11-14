//
// Loader version that supports multiboot specification.
//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2010, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "multiboot.h"
#include "bootinfo.h"
#include "elf_parser.h"
#include "new.h"
#include "default_console.h"
#include "debugger.h"
#include "module_loader.h"

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
    bootinfo_t* bi = new(BOOTINFO_PAGE) bootinfo_t(true);

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
        bi->append_module(i, _mbi->module(i));

    bi->append_cmdline(_mbi->cmdline());

    return true;
}


//*****************************************************************************************************************
// TODO:
// relocate modules to 16MiB area, including kernel code.
// record updated location info directly into bootinfo page entries.
// this will copy only needed code and data (e.g. booting without "debug" will not copy debug infos)
// and will also properly allocate space for .bss
// use module_loader instance for this
//*****************************************************************************************************************

/*!
 * Init function that understands multiboot info structure.
 *
 * The procedure goes as follows:
 * - We have mbi inside the bootinfo page already.
 * - ELF-load the kernel module.
 * @return entry point for the kernel.
 */
address_t mbi_init()
{
    kconsole << "mbi_init()" << endl;
    multiboot_t* mbi = multiboot_t::prepare();

    // relocate kernel-startup elf
    // offset of .text section from load address and offset of entry point from .text will give relocation offsets
    bootinfo_t* bi = new(BOOTINFO_PAGE) bootinfo_t;

    address_t start = mbi->module(0)->mod_start;
    elf_parser_t elf(start);

    // will move .text to 16MiB and the rest afterwards
    // should also relocate and return entry point address...
    void* entry = bi->get_module_loader().load_module("kernel-boot", elf, "entry");

//     elf32::section_header_t* text = elf.section_header(".text");

//     start = 16*MiB;
//     ptrdiff_t offset = start - text->addr + text->offset;
//     if (!elf.is_relocatable() && offset != 0)
//         PANIC("unrelocatable kernel-startup, cannot proceed.");
//     elf.relocate_to(start); // relocate in place, then copy over to new location:

//     address_t entry = elf.get_entry_point();

//     kconsole << "Kernel relocated by " << (address_t)offset << ", original entry at " << entry << endl;

    return (address_t)entry;// + offset;
}
