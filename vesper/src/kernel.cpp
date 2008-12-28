//
// Copyright 2007 - 2008, Stanislav Karchebnyy <berkus+metta@madfire.net>
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

namespace metta {
namespace kernel {

page_fault_handler page_fault_handler_;

void kernel::run()
{
	if (!multiboot.is_elf())
		PANIC("ELF information is missing in kernel!");

	// Make sure we aren't overwriting anything by writing at placementAddress.
	relocate_placement_address();

	kernel_elf_parser.load_kernel(multiboot.symtab_start(),
                                  multiboot.strtab_start());

	global_descriptor_table::init();

	interrupts_table.set_isr_handler(14, &page_fault_handler_);
	interrupts_table.init();

	memory_manager.init(multiboot.upper_mem() * 1024);
	memory_manager.remap_stack();
	kconsole.debug_log("Remapped stack and ready to rock.");

	task::init();
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
    for (unsigned int i = 0; i < multiboot.mod_count(); i++)
    {
        multiboot::modinfo *m = multiboot.mod(i);
        kconsole.print("Module %d @ %p to %p:\n", i+1, m->mod_start, m->mod_end);
        if (m->str)
            kconsole.print("       %s\n", m->str);
        if (!strcmp(m->str, "/initfs"))
        {
            kconsole.print("FOUND INITFS\n");
            initfsmod = m;
        }
        if (!strcmp(m->str, "/init"))
        {
            kconsole.print("FOUND INIT\n");
            initmod = m;
            // init is a statically linked elf file, load it and jump to entrypoint
            // init startup takes references to multiboot and initfs interfaces.
        }
    }

    multiboot.print_mmap_info();

    ASSERT(initfsmod && initmod);
//    initfs init_fs(initfsmod->start);
//    initcomp init_comp(initmod->start);

    // FIXME: create com_imultiboot and com_iinitfs interfaces and pass to init_comp
//    init_comp(multiboot, init_fs);

    while (1) {
        //thread::self()->set_name("kernel_idle");
        //asm("hlt");
//         scheduler::yield();
    }
}

#define MAX(a,b) ((a>b)?(a):(b))

void kernel::relocate_placement_address()
{
    address_t new_placement_address = memory_manager.get_placement_address();
    if (multiboot.is_elf())
    {
        new_placement_address = MAX(multiboot.symtab_end(), new_placement_address);
        new_placement_address = MAX(multiboot.strtab_end(), new_placement_address);
    }
    new_placement_address = MAX(multiboot.last_mod_end(), new_placement_address);
    memory_manager.set_placement_address(new_placement_address);
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
		kconsole.print_hex((unsigned int)ptr);
		kconsole.print("  ");
		run = size < 16 ? size : 16;
		for(int i = 0; i < run; i++)
		{
			kconsole.print_byte(*(ptr+i));
			kconsole.print_char(' ');
			if (i == 7)
				kconsole.print_char(' ');
		}
		if (run < 16)// pad
		{
			if(run < 8)
				kconsole.print_char(' ');
			for(int i = 0; i < 16-run; i++)
				kconsole.print("   ");
		}
		kconsole.print_char(' ');
		for(int i = 0; i < run; i++)
		{
			char c = *(ptr+i);
			if (c == kconsole.eol)
				c = ' ';
			kconsole.print_char(c);
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
	kconsole.set_color(GREEN);
	kconsole.print("*** Backtrace *** Tracing %d stack frames:\n", n);
	int i = 0;
    while (base_pointer && eip &&
		( (n && i<n) || !n) &&
		eip < 0x87000000)
	{
        base_pointer = backtrace(base_pointer, eip);
		unsigned int offset;
		char *symbol = kernel_elf_parser.find_symbol(eip, &offset);
		offset = eip - offset;
		kconsole.print("| %08x <%s+0x%x>\n", eip, symbol ? symbol : "UNRESOLVED", offset);
		i++;
	}
}

void kernel::print_stacktrace(unsigned int n)
{
    address_t esp = read_stack_pointer();
    address_t espBase = esp;
    kconsole.set_color(GREEN);
    kconsole.print("<ESP=%08x>\n", esp);
    for (unsigned int i = 0; i < n; i++)
    {
        kconsole.print("<ESP+%4d> %08x\n", esp - espBase, *(address_t*)esp);
        esp += sizeof(address_t);
    }
}

}
}

// kate: indent-width 4; replace-tabs on;
// vi:set ts=4:set expandtab=on:
