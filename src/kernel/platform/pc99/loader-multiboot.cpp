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

/*!
 * Check if a valid multiboot info structure is present.
 */
bool mbi_probe()
{
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


/**
 * Init function that understands multiboot info structure.
 *
 * The procedure goes as follows:
 * - We have mbi inside the bootinfo page already.
 * - ELF-load the kernel module.
 * - Flush caches.
 * - Launch the kernel.
 *
 * @returns entry point for kernel
 */
address_t mbi_init()
{
    multiboot_t* mbi = multiboot_t::prepare();

//     if (mbi->flags.cmdline)
//     {
//     char * p;
//     COPY_STRING (mbi->cmdline);
// 
/*#define PARSENUM(name, var, msg, massage...)            \
        if ((p = strstr(mbi->cmdline, name"=")) != NULL)    \
        {                           \
            var = strtoul(p+strlen(name)+1, &p, 10);        \
            if (*p == 'K') var*=1024;               \
            if (*p == 'M') var*=1024*1024;          \
            if (*p == 'G') var*=1024*1024*1024;         \
            massage;                        \
            printf(msg,                     \
                   var >= 1<<30 ? var>>30 :         \
                   var >= 1<<20 ? var>>20 :         \
                   var >= 1<<10 ? var>>10 : var,        \
                   var >= 1<<30 ? "G" :             \
                   var >= 1<<20 ? "M" :             \
                   var >= 1<<10 ? "K" : "");            \
        }*/
// 
/*#define PARSEBOOL(name, var, msg)               \
    if ((p = strstr (mbi->cmdline, name"=")) != NULL)   \
    {                           \
        p = strchr (p, '=') + 1;                \
        if (strncmp (p, "yes", 3) == 0 ||           \
        strncmp (p, "on", 2) == 0 ||            \
        strncmp (p, "enable", 6) == 0)          \
        {                           \
        if (! var) printf ("Enabling %s\n", msg);   \
        var = true;                 \
        }                           \
        else if (strncmp (p, "no", 2) == 0 ||       \
             strncmp (p, "off", 3) == 0 ||      \
             strncmp (p, "disable", 7) == 0)        \
        {                           \
        if (var) printf ("Disabling %s\n", msg);    \
        var = false;                    \
        }                           \
    }*/
// 
//         PARSENUM("maxmem",
//                  max_phys_mem,
//                  "Limiting physical memory to %d%sB\n");
//         PARSENUM("kmem",
//                  additional_kmem_size,
//                  "Reserving %d%sB for kernel memory\n",
//                  additional_kmem_size &= ~(kip.get_min_pagesize()-1));
// 
//     PARSEBOOL ("bootinfo", use_bootinfo, "generic bootinfo");
//     PARSEBOOL ("mbi", use_mbi, "multiboot info");
//     PARSEBOOL ("decode-all", decode_all_executables,
//            "decoding of all executables");
//     }
// 
//     // Load the first three modules as ELF images into memory
//     if (!load_modules())
//     {
//         // Bail out if loading failed
//         printf("Failed to load all necessary modules\n");
//         FAIL();
//     }
// 
//     // Protect all user-level modules.
//     for (L4_Word_t i = 3; i < mbi->modcount; i++)
//     kip.dedicate_memory (mbi->mods[i].start, mbi->mods[i].end - 1,
//                  L4_BootLoaderSpecificMemoryType,
//                  kip_manager_t::desc_boot_module);
// 

//     relocate kernel-startup elf
//     offset of .text section from load address and offset of entry point from .text will give relocation offsets

    address_t start = mbi->module(0)->mod_start;
    elf_parser_t elf(start);
    elf32::section_header_t* text = elf.section_header(".text");
    address_t entry = elf.get_entry_point();

    address_t offset = start - text->addr + text->offset;

    if (!elf.is_relocatable() && offset != 0)
        PANIC("unrelocatable kernel-startup, cannot proceed.");

    kconsole << "kernel startup module at " << start << endl
             << "text section in mem at " << text->addr << endl
             << "text section in file at " << text->offset << endl
             << "entry point at " << entry << endl
             << endl;

    elf.relocate(offset);

    debugger_t::dump_memory(entry + offset, 100);

    return entry + offset;
}
