//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "kernel.h"
#include "globals.h"
#include "registers.h"
#include "multiboot.h"
#include "elf_parser.h"
#include "memory_manager.h"
#include "default_console.h"
#include "global_descriptor_table.h"
#include "interrupt_descriptor_table.h"
#include "timer.h"
#include "task.h"
#include "page_fault_handler.h"
#include "atomic_count.h"
#include "memutils.h"

using metta::common::memutils;

namespace metta {
namespace kernel {

page_fault_handler page_fault_handler_;

void kernel::run()
{
    // NB: all this code could be in the kernel-bootstrapper module that is
    // intermediate and can be removed after it has finished the kernel initialisation.

    if (!multiboot::self().is_elf())
        PANIC("ELF information is missing in kernel!");

    critical_section(); // do not interrupt us in the following lines, please

    // Make sure we aren't overwriting anything by writing at placementAddress.
    relocate_placement_address();

    kernel_elf_parser.load_kernel(multiboot::self().symtab_start(),
                                  multiboot::self().strtab_start());

    global_descriptor_table::init();

    interrupts_table.set_isr_handler(14, &page_fault_handler_);
    interrupts_table.init();

    // Always pass in a meg of lower memory.
    kconsole << "Lower mem: " << multiboot::self().lower_mem() << endl
             << "Upper mem: " << multiboot::self().upper_mem() << endl;
    kmemmgr.init(1024 * 1024 + multiboot::self().upper_mem() * 1024, multiboot::self().memory_map());
    kmemmgr.remap_stack();
    kconsole.debug_log("Remapped stack and ready to rock.");

//    smp_boot(); // prepare per-cpu data area, boot remaining processors

    task::boot();
    timer::init();//crashes at start of timer init (stack problem?)
// tasking causes stack fuckups after timer inits and causes a yield?
// weird: seems to work now. check gcc optimizations.

    // Load init component and its initfs.
    // Pass control to init component in supervisor (user?) mode.
    // After that the kernel's startup business is over
    // and execution continues in the userspace root server.
    //
    // init+initfs comprise the libos loaded by the kernel to do actual things.

    multiboot::modinfo *initfsmod = 0;
    multiboot::modinfo *initmod = 0;
    for (unsigned int i = 0; i < multiboot::self().mod_count(); i++)
    {
        multiboot::modinfo *m = multiboot::self().mod(i);
        kconsole.print("Module %d @ %p to %p:\n", i+1, m->mod_start, m->mod_end);
        if (m->str)
            kconsole.print("       %s\n", m->str);
        if (!memutils::strcmp(m->str, "/initfs"))//FIXME: replace with string() method
        {
            kconsole.print("FOUND INITFS\n");
            initfsmod = m;
        }
        if (!memutils::strcmp(m->str, "/init"))//FIXME: replace with string() method
        {
            kconsole.print("FOUND INIT\n");
            initmod = m;
            // init is a statically linked elf file, load it and jump to entrypoint
            // init startup takes references to multiboot and initfs interfaces.
        }
    }

    ASSERT(initfsmod && initmod);
//    initfs init_fs(initfsmod->start);
//    initcomp init_comp(initmod->start);

    // FIXME: create com_imultiboot and com_iinitfs interfaces and pass to init_comp
//    init_comp(multiboot, init_fs);

    end_critical_section(); // now you can interrupt us and multitask and everything else

    // init root kernel thread and upcall into init component.
//     thread *root_thread = new thread;
//     root_thread->upcall();

    while (1) {
        //thread::self()->set_name("kernel_idle");
        //asm("hlt");
//         scheduler::yield();
    }
}

void kernel::relocate_placement_address()
{
    address_t new_placement_address = kmemmgr.get_placement_address();
    if (multiboot::self().is_elf())
    {
        new_placement_address = max(multiboot::self().symtab_end(), new_placement_address);
        new_placement_address = max(multiboot::self().strtab_end(), new_placement_address);
    }
    new_placement_address = max(multiboot::self().last_mod_end(), new_placement_address);
    kmemmgr.set_placement_address(new_placement_address);
}

void kernel::dump_memory(address_t start, size_t size)
{
    char *ptr = (char *)start;
    int run;

    // Silly limitation, probably.
    if (size > 256)
        size = 256;

    kconsole.newline();

    while (size > 0)
    {
        kconsole.print((unsigned int)ptr);
        kconsole.print("  ");
        run = size < 16 ? size : 16;
        for(int i = 0; i < run; i++)
        {
            kconsole.print(*(ptr+i));
            kconsole.print(' ');
            if (i == 7)
                kconsole.print(' ');
        }
        if (run < 16)// pad
        {
            if(run < 8)
                kconsole.print(' ');
            for(int i = 0; i < 16-run; i++)
                kconsole.print("   ");
        }
        kconsole.print(' ');
        for(int i = 0; i < run; i++)
        {
            char c = *(ptr+i);
            if (c == kconsole.eol)
                c = ' ';
            kconsole.print(c);
        }
        kconsole.newline();
        ptr += run;
        size -= run;
    }
}

address_t kernel::backtrace(address_t base_pointer, address_t& return_address)
{
    // We take a stack base pointer (in basePointer), return what it's pointing at
    // and put the Address just above it in the stack in returnAddress.
    address_t next_base = *((address_t*)base_pointer);
    return_address    = *((address_t*)(base_pointer+sizeof(address_t)));
    return next_base;
}

address_t kernel::backtrace(int n)
{
    address_t base_pointer = read_base_pointer();
    address_t eip = 1;
    int i = 0;
    while (base_pointer && eip /*&& eip < 0x87000000*/)
    {
        base_pointer = backtrace(base_pointer, eip);
        if (i == n)
        {
            return eip;
        }
        i++;
    }
    return 0;
}

void kernel::print_backtrace(address_t base_pointer, int n)
{
    address_t eip = 1; // Don't initialise to 0, will kill the loop immediately.
    if (base_pointer == 0)
    {
        base_pointer = read_base_pointer();
    }
    kconsole << GREEN << "*** Backtrace *** Tracing ";
    if (n == 0)
        kconsole << "all";
    else
        kconsole << n;
    kconsole << " stack frames:" << endl;
    int i = 0;
    while (base_pointer && eip &&
        ( (n && i<n) || !n) &&
        eip < 0x87000000)
    {
        base_pointer = backtrace(base_pointer, eip);
        unsigned int offset;
        char *symbol = kernel_elf_parser.find_symbol(eip, &offset);
        offset = eip - offset;
        kconsole << "| " << (unsigned)eip << " <" << (symbol ? symbol : "UNRESOLVED") << "+0x" << offset << ">" << endl;
        i++;
    }
}

void kernel::print_stacktrace(unsigned int n)
{
    address_t esp = read_stack_pointer();
    address_t espBase = esp;
    kconsole << GREEN << "<ESP=" << esp << ">" << endl;
    for (unsigned int i = 0; i < n; i++)
    {
        kconsole << "<ESP+" << (int)(esp - espBase) << "> " << *(address_t*)esp << endl;
        esp += sizeof(address_t);
    }
}

}
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
