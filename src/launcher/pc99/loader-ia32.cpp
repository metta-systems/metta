//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2011, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Arch-specific part of the launcher.
//
#include "loader.h"
#include "continuation.h"
#include "registers.h"
#include "segs.h"

/*
* Functions needed for loader format structure.
*/
bool mbi_probe();
address_t mbi_prepare();

/*
 * Loader formats supported for IA32.
 */
loader_format_t loader_formats[] = {
    { "multiboot compliant loader", mbi_probe, mbi_prepare },
    NULL_LOADER
};


void flush_cache()
{
    __asm__ __volatile__ ("wbinvd");
}

/*!
 * Start bootstrapper at its entry point. No preconditions.
 * This function will perform a switch from ring0 to ring3.
 */
void launch_kernel(address_t entry)
{
	static continuation_t new_context; // non-static may work too

    // Create an execution context and activate it.
    continuation_t::gpregs_t gpregs;
    gpregs.esp = read_stack_pointer();
    gpregs.eax = 0;
    gpregs.ebx = 0;
    // gpregs.eflags = 0x03002; /* Flags for root domain: interrupts disabled, IOPL=3 (program can use IO ports) */
    gpregs.eflags = 0x03202; /* Flags for testing: interrupts enabled, IOPL=3 (program can use IO ports) */
    new_context.set_gpregs(gpregs);
    new_context.set_entry(entry, USER_CS, USER_DS);//FIXME: depends on gpregs being set before this call!
    // -- THIS IS WHERE RING0 ENDS --
    new_context.activate(); // we have a liftoff! root domain executes in ring3 just like everybody else.
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
