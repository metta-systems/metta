//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2012, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Hosted kernel startup initialisation.
//
#include "config.h"

#include "default_console.h"
#include "bootinfo.h"
#include "bootimage.h"
#include "infopage.h"
#include "frames_module_v1_interface.h"
#include "timer_v1_interface.h"
#include "continuation.h"
#include "cpu.h"
#include "c++ctors.h"
#include "gdt.h"
#include "idt.h"
#include "isr.h"
#include "root_domain.h"
#include "registers.h"
#include "new.h"
#include "debugger.h"

static void parse_cmdline(bootinfo_t* bi)
{
    const char* cmdline;

    if (bi->get_cmdline(cmdline))
    {
        kconsole << "Command line is: " << cmdline << endl;
    }
}

static void SECTION(".init.cpu") check_cpu_features()
{
    // TODO: request CPU features support from OS?
    uint32_t req_features = 0;
    uint32_t avail_features = 0;//x86_cpu_t::features();

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
static continuation_t new_context;

/**
 * Get the system going.
 *
 * Prepare all system-specific structures and initialise BP and APs. Enter root domain and continue there.
 *
 * TODO: relate Pistachio SMP startup routines here.
 */
extern "C" void kernel_startup()
{
    // No dynamic memory allocation here yet, global objects not constructed either.
    run_global_ctors();

    global_descriptor_table_t gdt;
    kconsole << "Created GDT." << endl;
    interrupt_descriptor_table_t::instance().install();
    kconsole << "Created IDT." << endl;

    // Grab the bootinfo page and discover where is our bootimage.
    bootinfo_t* bi = new(bootinfo_t::ADDRESS) bootinfo_t;

    address_t start, end;
    const char* name;
    if (!bi->get_module(1, start, end, name))
    {
        PANIC("Bootimage not found!");
    }

    bootimage_t bootimage(name, start, end);

    parse_cmdline(bi);
    prepare_infopage(); // <-- init domain info page
    check_cpu_features(); // cmdline might affect used CPU feats? (i.e. noacpi flag)
    
    // TODO: CREATE INITIAL MEMORY MAPPINGS PROPERLY HERE
    // TEMPORARY: just map all mem 0..min(16Mb, RAMtop) to 1-1 mapping? for simplicity
//    int ramtop = 16*MiB;
    bi->append_vmap(0, 0, 16*MiB);//min(16*MiB, ramtop));

    timer_v1::closure_t* timer = init_timer();
    timer->enable(0); // enable timer interrupts
    kconsole << "Timer interrupt enabled." << endl;
    x86_cpu_t::enable_fpu();
    kconsole << "FPU enabled." << endl;

//    kconsole << WHITE << "...in the living memory of V2_OS" << LIGHTGRAY << endl;

    root_domain_t root_dom(bootimage);

//     kconsole << "+ root_domain entry @ 0x" << root_dom.entry() << endl;

    // Create an execution context and activate it.
    continuation_t::gpregs_t gpregs;
    gpregs.esp = read_stack_pointer();
    gpregs.eax = 0;
    gpregs.ebx = 0;
    gpregs.eflags = 0x03002; /* Flags for root domain: interrupts disabled, IOPL=3 (program can use IO ports) */
    new_context.set_gpregs(gpregs);
    new_context.set_entry(root_dom.entry());//FIXME: depends on gpregs being set before this call!
    new_context.activate(); // we have a liftoff!

    /* Never reached */
    PANIC("root domain returned!");
}
