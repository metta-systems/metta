//
// Kernel startup initialisation.
//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2010, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "config.h"

#include "default_console.h"
#include "bootinfo.h"
#include "bootimage.h"

#include "memutils.h"
#include "memory.h"
#include "multiboot.h"
#include "gdt.h"
#include "idt.h"
#include "elf_parser.h"
#include "registers.h"
#include "c++ctors.h"
#include "debugger.h"
#include "linksyms.h"
#include "frame.h"
#include "page_directory.h"
#include "x86_frame_allocator.h"
#include "x86_protection_domain.h"
#include "new.h"
// #include "stretch_driver.h"

// Declare C linkage.
extern "C" void kernel_startup();
// extern "C" address_t placement_address;
// extern "C" address_t KICKSTART_BASE;

#if 0
static void map_identity(const char* caption, address_t start, address_t end)
{
#if MEMORY_DEBUG
    kconsole << "Mapping " << caption << endl;
#endif
    end = page_align_up<address_t>(end); // one past end
    for (uint32_t k = start/PAGE_SIZE; k < end/PAGE_SIZE; k++)
        protection_domain_t::privileged().map(k * PAGE_SIZE, reinterpret_cast<void*>(k * PAGE_SIZE), page_t::kernel_mode | page_t::writable);
}
#endif

static void parse_cmdline(bootinfo_t* bi)
{
    const char* cmdline;

    if (bi->get_cmdline(cmdline))
    {
        kconsole << "Command line is: " << cmdline << endl;
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
    }
}

//     // Protect all user-level modules.
//     for (L4_Word_t i = 3; i < mbi->modcount; i++)
//     kip.dedicate_memory (mbi->mods[i].start, mbi->mods[i].end - 1,
//                  L4_BootLoaderSpecificMemoryType,
//                  kip_manager_t::desc_boot_module);

/*!
 * Get the system going.
 *
 * TODO: relate Pistachio SMP startup routines here.
 */
void kernel_startup()
{
    // No dynamic memory allocation here yet, global objects not constructed either.

///    x86_frame_allocator_t::set_allocation_start(page_align_up<address_t>(std::max(LINKSYM(placement_address), bootimage->mod_end)));
    // now we can allocate memory frames

    bochs_console_print_str("Entering kernel-startup\n");

    kconsole << "Run global ctors" << endl;
    run_global_ctors();

//     setup_gdt();
//     setup_idt();
//     setup_fpu();
//     setup_pic();

    kconsole << "Create bootinfo" << endl;
    // Grab the BOOTPAGE and discover where is our nexus.
    bootinfo_t* bi = new(BOOTINFO_PAGE) bootinfo_t(false);

    kconsole << "Get boot module" << endl;
    address_t start, end;
    const char* name;
    if (bi->get_module(1, start, end, name))
    {
        kconsole << "Nexus at " << start << " till " << end << " named " << name << endl;
    }

    bootimage_t nexus(/*name,*/ start/*, end*/);

    parse_cmdline(bi);
//     get_cpuid();
//     init_cpu_features();
//         init_cache();
//         init_pmctr();
//     prepare_infopage();
//     Timer$Enable();
//     init_mem();
//     enable_fpu();
//     k_presume(RootDomain);

///    x86_frame_allocator_t::instance().initialise_before_paging(mb.memory_map(), x86_frame_allocator_t::instance().reserved_range());
    // now we can also free dynamic memory

#if MEMORY_DEBUG
/**    kconsole << GREEN << "lower mem = " << (int)mb.lower_mem() << "KB" << endl
                      << "upper mem = " << (int)mb.upper_mem() << "KB" << endl;

    kconsole << WHITE << "We are loaded at " << LINKSYM(KICKSTART_BASE) << endl
                      << "    bootimage at " << bootimage->mod_start << ", end " << bootimage->mod_end << endl;*/
#endif
#if 0
    // Identity map currently executing code.
    // page 0 is not mapped to catch null pointers
    map_identity("bottom 4Mb", PAGE_SIZE, 4*MiB - PAGE_SIZE);

    global_descriptor_table_t gdt;
    kconsole << "Created gdt." << endl;

//     stretch_driver_t::default_driver().initialise();
    interrupt_descriptor_table_t::instance().install();
    kconsole << "Created idt." << endl;

    static_cast<x86_protection_domain_t&>(protection_domain_t::privileged()).enable_paging();
    // now we have paging enabled.
    kconsole << "Enabled paging." << endl;

    // Load the modules.
    // Module "boot" depends on all modules that must be probed at startup.
    // Dependency resolution will bring up modules in an appropriate order.
//    load_modules(bootimage, "boot");

    // Load components from bootcp.
//     kconsole << "opening initfs @ " << bootimage->mod_start << endl;
//     initfs_t initfs(bootcp->mod_start);
//     typedef void (*comp_entry)(bootinfo_t bi_page);

    // For now, boot components are all linked at different virtual addresses so they don't overlap.
//     kconsole << "iterating components" << endl;
//     for (uint32_t k = 0; k < initfs.count(); k++)
//     {
//         kconsole << YELLOW << "boot component " << initfs.get_file_name(k) << " @ " << initfs.get_file(k) << endl;
//         // here loading an image should create a separate PD with its own pagedir
// //         domain_t* d = nucleus->create_domain();
// 
//         if (!elf.load_image(initfs.get_file(k), initfs.get_file_size(k)))
//             kconsole << RED << "not an ELF file, load failed" << endl;
//         else
//             kconsole << GREEN << "ELF file loaded, entering component init" << endl;
//         comp_entry init_component = (comp_entry)elf.get_entry_point();
//         init_component(bootinfo);
//     }

    // TODO: instantiate kernel interfaces

    kconsole << WHITE << "...in the living memory of V2_OS" << endl;

    // TODO: run a predefined root_server_entry portal here
    kconsole << "Allocating frame: " << frame_allocator_t::instance().allocate_frame() << endl;

    /* Never reached */
    PANIC("root_server returned!");
#endif
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
