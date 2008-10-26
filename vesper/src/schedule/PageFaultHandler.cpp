//
// Copyright 2007 - 2008, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "DefaultConsole.h"
#include "Globals.h"
#include "Kernel.h"
#include "common.h"
#include "PageFaultHandler.h"

// Interrupts are disabled upon entry to run()
void PageFaultHandler::run(Registers *r)
{
	PANIC("Page fault");
	// A page fault has occurred.
	// The faulting address is stored in the CR2 register.
	uint32_t faulting_address;
	asm volatile("mov %%cr2, %0" : "=r" (faulting_address));

	// The error code gives us details of what happened.
	bool present  = (r->err_code & 0x1) ? true : false; // Page not present
	bool rw       = (r->err_code & 0x2) ? true : false; // Write operation?
	bool us       = (r->err_code & 0x4) ? true : false; // Processor was in user-mode?
	bool reserved = (r->err_code & 0x8) ? true : false; // Overwritten CPU-reserved bits of page entry?
	bool insn     = (r->err_code & 0x10) ? true : false; // Caused by an instruction fetch?

	// Output an error message.
	kconsole.set_attr(LIGHTRED, BLACK);
	kconsole.print("Page fault! at EIP=%08x, faulty address=%08x( ", r->eip, faulting_address);
	kconsole.set_attr(WHITE, BLACK);
	// See intel manual -- p = 0 if page fault is due to a nonpresent page.
	if (!present) kconsole.print("Page not present ");
	if (rw) kconsole.print("Write to read-only memory ");
	if (us) kconsole.print("In user-mode ");
	if (reserved) kconsole.print("Overwritten reserved bits ");
	if (insn) kconsole.print("Instruction fetch ");
	kconsole.set_attr(LIGHTCYAN, BLACK);
	kconsole.print(")\n");
// 	kernel.printBacktrace();
	PANIC("Page fault");
}
// kate: indent-width 4; replace-tabs on;
// vi:set ts=4:set expandtab=on:
