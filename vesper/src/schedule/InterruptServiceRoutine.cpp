//
// Copyright 2007 - 2008, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "DefaultConsole.h"
#include "InterruptServiceRoutine.h"
#include "InterruptDescriptorTable.h"
#include "Globals.h"
#include "common.h"

extern "C"
{
	void isrHandler(Registers regs);
	void irqHandler(Registers regs);
}

// Handles a software interrupt/CPU exception.
// This is architecture specific!
// This gets called from our asm interrupt handler stub.
// TODO: implement handling from usermode.
void isrHandler(Registers regs)
{
	kconsole << GREEN << "Received interrupt: " << regs.int_no << endl;

	InterruptServiceRoutine *isr = interruptsTable.getIsr(regs.int_no);
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
void irqHandler(Registers regs)
{
// 	kconsole << GREEN << "Received irq: " << regs.int_no << endl;

	InterruptServiceRoutine *isr = interruptsTable.getIsr(regs.int_no);
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
