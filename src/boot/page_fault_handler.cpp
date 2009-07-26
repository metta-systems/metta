//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "default_console.h"
#include "globals.h"
#include "kernel.h"
#include "common.h"
#include "page_fault_handler.h"

static bool test_flag(int flag, int mask)
{
    return (flag & mask) ? true : false;
}

// Interrupts are disabled upon entry to run()
void page_fault_handler::run(registers *r)
{
// 	PANIC("Page fault");
	// A page fault has occurred.
	// The faulting address is stored in the CR2 register.
	uint32_t faulting_address;
	asm volatile("mov %%cr2, %0" : "=r" (faulting_address));

	// The error code gives us details of what happened.
	bool present  = test_flag(r->err_code, 0x01); // Page not present
	bool rw       = test_flag(r->err_code, 0x02); // Write operation?
	bool us       = test_flag(r->err_code, 0x04); // Processor was in user-mode?
	bool reserved = test_flag(r->err_code, 0x08); // Overwritten CPU-reserved bits of page entry?
	bool insn     = test_flag(r->err_code, 0x10); // Caused by an instruction fetch?

	// Output an error message.
	kconsole.set_attr(LIGHTRED, BLACK);
	kconsole.print("Page fault! at EIP=%x, faulty address=%x (", r->eip, faulting_address);
	kconsole.set_attr(WHITE, BLACK);
	// See intel manual -- p = 0 if page fault is due to a nonpresent page.
	if (!present) kconsole.print("Page not present");
	if (rw) kconsole.print(" Write to read-only memory");
	if (us) kconsole.print(" In user-mode");
	if (reserved) kconsole.print(" Overwritten reserved bits");
	if (insn) kconsole.print(" Instruction fetch");
	kconsole.set_attr(LIGHTCYAN, BLACK);
	kconsole.print(")\n");
// 	kernel.printBacktrace();
	PANIC("Page fault");
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
