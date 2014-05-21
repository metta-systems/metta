//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2012, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Kernel startup initialisation.
//
#include "config.h"

#include "default_console.h"
#include "bootinfo.h"
#include "infopage.h"
#include "frames_module_v1_interface.h"
#include "timer_v1_interface.h"
#include "mmu.h"
#include "c++ctors.h"
#include "new"
#include "debugger.h"
#include "logger.h"

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
        if ((p = strstr(mbi->cmdline, name"=")) != nullptr)    \
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
    if ((p = strstr (mbi->cmdline, name"=")) != nullptr)   \
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

static void check_cpu_features() SECTION(".init.cpu")
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

//     bochs:
//     CPU does not support all required features 0xffffffff (? psn ? ds acpi ss ht tm ia64 pbe missing)

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
        {
            if ((req_features & 1 << i) && (!(avail_features & 1 << i))) {
                kconsole << x86_32_features[i] << " ";
            }
        }
        kconsole << "missing)" << endl;
        PANIC("Unsupported CPU!");
    }

    uint32_t max_cpuid, family, dummy;

    if (x86_cpu_t::has_cpuid()) {
        x86_cpu_t::cpuid(0, &max_cpuid, &dummy, &dummy, &dummy);
    }
    else {
        max_cpuid = 0;
    }

    // kconsole << "max cpuid level " << max_cpuid << endl;
    x86_cpu_t::cpuid(0x80000001, &dummy, &dummy, &dummy, &family);
    if (family & (1 << 29)) {
        kconsole << "64 bits cpu supported! " << family << endl;
    }

#define MSR_EFER 0xC0000080
#define MSR_EFER_SCE (1 << 0)
#define MSR_EFER_LME (1 << 8)

    // Switch from protected mode to Long Mode.
    // ia32_mmu_t::disable_paged_mode(); // Disable paging
    // debugger_t::breakpoint();
    // ia32_mmu_t::enable_4mb_pages();
    // ia32_mmu_t::enable_global_pages();
    // ia32_mmu_t::enable_2mb_pages(); // Set the PAE enable bit in CR4
    // address_t* PML2 = (address_t*)0x1000;
    // address_t* PML3 = (address_t*)0x2000;
    // address_t* PML4 = (address_t*)0x3000;
    // *PML2 = 0x87; // single 4Mb ID mapping
    // *PML3 = 0x1000 | 7;
    // *PML4 = 0x2000 | 7;
    // ia32_mmu_t::set_active_pagetable(0x3000); // Load CR3 with the physical address of the PML4
    // x86_cpu_t::write_msr(MSR_EFER, x86_cpu_t::read_msr(MSR_EFER) | MSR_EFER_LME | MSR_EFER_SCE); // Enable long mode by setting the EFER.LME flag in MSR 0xC0000080
    // ia32_mmu_t::enable_paged_mode(); // Enable paging
// Now the CPU will be in compatibility mode, and instructions are still 32-bit.
// To enter long mode, the D/B bit (bit 22, 2nd dword) of the GDT code segment
// must be clear (as it would be for a 16-bit code segment), and
// the L bit (bit 21, 2nd dword) of the GDT code segment must be set.
// Once that is done, the CPU is in 64-bit long mode.
    // load_64_gdt();
    // load_64_idt();
    // Reload segment registers.
    // debugger_t::breakpoint();

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

    INFO_PAGE.cpu_features = avail_features;
}

/* Clear out the information page */
static void prepare_infopage()
{
    INFO_PAGE.pervasives = 0;
    INFO_PAGE.scheduler_heartbeat = 0; // Scheduler passes
    INFO_PAGE.irqs_heartbeat      = 0; // IRQ calls
    INFO_PAGE.glue_heartbeat      = 0; // glue code calls
    INFO_PAGE.faults_heartbeat    = 0; // protection faults
    INFO_PAGE.cpu_features        = 0;
}

extern timer_v1::closure_t* init_timer(); // YIKES external declaration! FIXME

/**
 * Get the system going.
 *
 * Prepare all system-specific structures and initialise BP and APs.
 *
 * @todo Relate Pistachio SMP startup routines here.
 */
extern "C" void arch_prepare()
{
    // No dynamic memory allocation here yet, global objects not constructed either.
    run_global_ctors();

    logger::function_scope fs("arch_prepare");

    // Grab the bootinfo page and discover where is our bootimage.
    bootinfo_t* bi = new(bootinfo_t::ADDRESS) bootinfo_t;

    parse_cmdline(bi);
    prepare_infopage(); // <-- init domain info page
    check_cpu_features(); // cmdline might affect used CPU feats? (i.e. noacpi flag)

    // TODO: CREATE INITIAL MEMORY MAPPINGS PROPERLY HERE
    // TEMPORARY: just map all mem 0..min(16Mb, RAMtop) to 1-1 mapping? for simplicity
    int ramtop = 32*MiB;
    bi->append_vmap(0, 0, ramtop);

    // @todo Timer interrupt should be enabled by the scheduler module once it installs the timer IRQ handler...
    // timer_v1::closure_t* timer = init_timer();
    // timer->enable(0); // enable timer interrupts
    // kconsole << "Timer interrupt enabled." << endl;

    // FPU should be enabled here orly?
    x86_cpu_t::enable_fpu();
    kconsole << "FPU enabled." << endl;
}
