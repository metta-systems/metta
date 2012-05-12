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
#include "cpu.h"
#include "c++ctors.h"
#include "new.h"

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

class func_logging_aid_t
{
    const char* name;
public:
    func_logging_aid_t(const char* _name) : name(_name) { kconsole << name << " {" << endl; }
    ~func_logging_aid_t() { kconsole << "} " << name << endl; }
};

/*!
 * Get the system going.
 *
 * Prepare all system-specific structures and initialise BP and APs. Enter root domain and continue there.
 *
 * TODO: relate Pistachio SMP startup routines here.
 */
extern "C" void arch_prepare()
{
    // No dynamic memory allocation here yet, global objects not constructed either.
    run_global_ctors();

    func_logging_aid_t aid("arch_prepare");

    // Grab the bootinfo page and discover where is our bootimage.
    bootinfo_t* bi = new(bootinfo_t::ADDRESS) bootinfo_t;

    parse_cmdline(bi);
    prepare_infopage(); // <-- init domain info page
    check_cpu_features(); // cmdline might affect used CPU feats? (i.e. noacpi flag)
    
    // TODO: CREATE INITIAL MEMORY MAPPINGS PROPERLY HERE
    // TEMPORARY: just map all mem 0..min(16Mb, RAMtop) to 1-1 mapping? for simplicity
//    int ramtop = 16*MiB;
    bi->append_vmap(0, 0, 16*MiB);//min(16*MiB, ramtop));

    // timer_v1::closure_t* timer = init_timer();
    // timer->enable(0); // enable timer interrupts
    // kconsole << "Timer interrupt enabled." << endl;

    // FPU should be enabled here orly?
    x86_cpu_t::enable_fpu();
    kconsole << "FPU enabled." << endl;
}
