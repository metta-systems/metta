//
// Kernel startup initialisation.
//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2010, Stanislav Karchebnyy <berkus@exquance.com>
// Code portions copyright 2002-2008, 2010, Karlsruhe University
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

        // supported cmdline params:
        // "debug"
        // "noapic"
        // "maxmem="

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

//         PARSENUM("maxmem",
//                  max_phys_mem,
//                  "Limiting physical memory to %d%sB\n");
//         PARSENUM("kmem",
//                  additional_kmem_size,
//                  "Reserving %d%sB for kernel memory\n",
//                  additional_kmem_size &= ~(kip.get_min_pagesize()-1));
    }
}

//     // Protect all user-level modules.
//     for (L4_Word_t i = 3; i < mbi->modcount; i++)
//     kip.dedicate_memory (mbi->mods[i].start, mbi->mods[i].end - 1,
//                  L4_BootLoaderSpecificMemoryType,
//                  kip_manager_t::desc_boot_module);

static void SECTION(".init.cpu") check_cpu_features()
{
    uint32_t req_features = X86_32_FEAT_FPU;
#if CONFIG_X86_PSE
    req_features |= X86_32_FEAT_PSE;
#endif
#if CONFIG_X86_PGE
    req_features |= X86_32_FEAT_PGE;
#endif
#if CONFIG_X86_FXSR
    req_features |= X86_32_FEAT_FXSR;
#endif
#if CONFIG_X86_SYSENTER
    req_features |= X86_32_FEAT_SEP;
#endif
#if CONFIG_IOAPIC
    req_features |= X86_32_FEAT_APIC;
#endif

    uint32_t avail_features = x86_cpu_t::features();

    if ((req_features & avail_features) != req_features)
    {
        kconsole << "CPU does not support all required features " << req_features << " (";
        const char* x86_32_features[] = {
            "fpu",  "vme",    "de",   "pse",   "tsc",  "msr", "pae",  "mce",
            "cx8",  "apic",   "?",    "sep",   "mtrr", "pge", "mca",  "cmov",
            "pat",  "pse-36", "psn",  "cflsh", "?",    "ds",  "acpi", "mmx",
            "fxsr", "sse",    "sse2", "ss",    "ht",   "tm",  "ia64", "pbe"
        };
        for (int i = 0; i < 32; i++)
            if ((req_features & 1 << i) && (!(avail_features & 1 << i)))
                kconsole << x86_32_features[i] << " ";
        kconsole << "missing)" << endl;
        PANIC("unsupported CPU!");
    }

    uint32_t max_cpuid, family, dummy;

    if (x86_cpu_t::has_cpuid())
        x86_cpu_t::cpuid(0, &max_cpuid, &dummy, &dummy, &dummy);
    else
        max_cpuid = 0;

    if (max_cpuid >= 1)
    {
        x86_cpu_t::cpuid(1, &family, &dummy, &dummy, &dummy);
        family = (family >> 8) & 0xf;
    }
    else
    {
        family = 0;
    }

    if (avail_features & X86_32_FEAT_PSE)
    {
        kconsole << "Enabling page size extension" << endl;
        ia32_mmu_t::enable_4mb_pages();
    }

    if (avail_features & X86_32_FEAT_PGE)
    {
        kconsole << "Enabling global pages" << endl;
        ia32_mmu_t::enable_global_pages();
    }

    /* If we have a 486 or above enable alignment checking */
    if (family >= 4)
    {
        kconsole << "Enabling alignment checking" << endl;
        x86_cpu_t::enable_alignment_checks();
    }

    /* If we have a Pentium or above enable the cache */
    if (family >= 5)
    {
        kconsole << "Enabling cache" << endl;
        x86_cpu_t::init_cache();
    }

    /* If we have a PPro or above, enable user-level reading of PMCTRs */
    /* Not supported in bochs. */
    if (family >= 6)
    {
        kconsole << "Enabling performance counters" << endl;
        x86_cpu_t::init_pmctr();
        x86_cpu_t::enable_user_pmctr();
    }
}

//--infopage.h--
struct information_page_t
{
    void* pervasives;
    uint64_t scheduler_heartbeat, irqs_heartbeat, glue_heartbeat, faults_heartbeat;
};

#define INFO_PAGE (*((information_page_t*)0x1000))
//--infopage.h--

/* Clear out the information page */
static void prepare_infopage()
{
    INFO_PAGE.pervasives = 0;
    INFO_PAGE.scheduler_heartbeat = 0; // Scheduler passes
    INFO_PAGE.irqs_heartbeat      = 0; // IRQ calls
    INFO_PAGE.glue_heartbeat      = 0; // glue code calls
    INFO_PAGE.faults_heartbeat    = 0; // protection faults
}

// setup gdt and page tables
static void init_mem()
{
    // create physical frames allocator
    // initialize physical memory
///    x86_frame_allocator_t::instance().initialise_before_paging(mb.memory_map(), x86_frame_allocator_t::instance().reserved_range());
    // create virtual memory allocator
    // initialize virtual memory map
    // create stretch allocator
    // assign stretches to address ranges
    // install page fault handler
    // enable paging

#if 0
    // Identity map currently executing code.
    // page 0 is not mapped to catch null pointers
    map_identity("bottom 4Mb", PAGE_SIZE, 4*MiB - PAGE_SIZE);
//     stretch_driver_t::default_driver().initialise();
    static_cast<x86_protection_domain_t&>(protection_domain_t::privileged()).enable_paging();
    // now we have paging enabled.
    kconsole << "Enabled paging." << endl;
#endif
//    x86_frame_allocator_t::set_allocation_start(page_align_up<address_t>(std::max(LINKSYM(placement_address), bootimage->mod_end)));
    // now we can allocate memory frames
}

static void load_modules(UNUSED_ARG bootimage_t& bm, UNUSED_ARG const char* root_module)
{
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
}

/*!
 * Get the system going.
 *
 * TODO: relate Pistachio SMP startup routines here.
 */
void kernel_startup()
{
    // No dynamic memory allocation here yet, global objects not constructed either.

    run_global_ctors();

    global_descriptor_table_t gdt;
    kconsole << "Created gdt." << endl;
    interrupt_descriptor_table_t::instance().install();
    kconsole << "Created idt." << endl;

    // Grab the bootinfo page and discover where is our bootimage.
    bootinfo_t* bi = new(BOOTINFO_PAGE) bootinfo_t(false);

    address_t start, end;
    const char* name;
    if (!bi->get_module(1, start, end, name))
    {
        PANIC("Bootimage not found!");
    }

    bootimage_t bootimage(name, start, end);

    parse_cmdline(bi);
    check_cpu_features(); // cmdline might affect used CPU feats? (i.e. noacpi flag)
    prepare_infopage(); // <-- init domain info page
    //timer_mod_cl = TimerMod$New(); // create timer instance
//     Timer$Enable(); // enable timer interrupts
    init_mem();
    x86_cpu_t::enable_fpu();

    // Load the modules.
    // Module "boot" depends on all modules that must be probed at startup.
    // Dependency resolution will bring up modules in an appropriate order.
    load_modules(bootimage, "boot");

    kconsole << WHITE << "...in the living memory of V2_OS" << endl;
//     k_presume(RootDomain); // we have a liftoff!

    /* Never reached */
    PANIC("root domain returned!");
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
