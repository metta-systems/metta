//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2011, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Arch-specific part of the loader.
//
#include "loader.h"

/*
* Functions needed for loader format structure.
*/
bool mbi_probe();
address_t mbi_init();

/*
 * Loader formats supported for IA32.
 */
loader_format_t loader_formats[] = {
    { "multiboot compliant loader", mbi_probe, mbi_init },
    NULL_LOADER
};


void flush_cache()
{
    __asm__ __volatile__ ("wbinvd");
}

/*
 * Start kernel at its entry point. No preconditions
 */
void launch_kernel(address_t entry)
{
    __asm__ __volatile__ ("jmp *%0" : : "r"(entry));
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
