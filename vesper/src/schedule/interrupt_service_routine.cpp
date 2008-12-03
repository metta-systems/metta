//
// Copyright 2007 - 2008, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "default_console.h"
#include "interrupt_service_routine.h"
#include "interrupt_descriptor_table.h"
#include "globals.h"
#include "common.h"

extern "C"
{
	void isr_handler(metta::kernel::Registers regs);
    void irq_handler(metta::kernel::Registers regs);
}

using namespace metta::kernel;

// Handles a software interrupt/CPU exception.
// This is architecture specific!
// This gets called from our asm interrupt handler stub.
// TODO: implement handling from usermode.
//
void isr_handler(Registers regs)
{
	kconsole << GREEN << "Received interrupt: " << regs.int_no << endl;

    interrupt_service_routine* isr = interrupts_table.getIsr(regs.int_no);
	if (isr)
	{
		isr->run(&regs);
	}
}

// IRQ8 and above should be acknowledged to the slave controller, too.
#define SLAVE_IRQ 40

// Handles a hardware interrupt request.
// This is architecture specific!
// This gets called from our asm hardware interrupt handler stub.
//
void irq_handler(Registers regs)
{
	kconsole << GREEN << "Received irq: " << regs.int_no-32 << endl;

    interrupt_service_routine* isr = interrupts_table.getIsr(regs.int_no);
	if (isr)
	{
		isr->run(&regs);
	}

	// Send an EOI (end of interrupt) signal to the PICs.
	// If this interrupt involved the slave.
	if (regs.int_no >= SLAVE_IRQ)
	{
		// Send reset signal to slave.
		outb(0xA0, 0x20);
	}
	// Send reset signal to master. (As well as slave, if necessary).
	outb(0x20, 0x20);
}

// kate: indent-width 4; replace-tabs on;
// vi:set ts=4:set expandtab=on:
