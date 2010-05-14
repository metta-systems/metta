//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2010, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "loader.h"
#include "panic.h"
#include "debugger.h"

/*!
 * Main kickstart loader function. Parses through all loader formats to find a valid one.
 */
extern "C" void loader()
{
//     run_global_ctors();

    bochs_console_print_str("loader()\n");
    loader_format_t* format = NULL;

    for (size_t n = 0; loader_formats[n].probe; ++n)
    {
        if (loader_formats[n].probe())
        {
            format = &loader_formats[n];
            break;
        }
    }

    if (format == NULL)
    {
        PANIC("No valid loader format found.");
    }

    bochs_console_print_str(format->name);
    bochs_console_print_str(" ->init()\n");
    address_t entry = format->init();

    if (!entry)
        PANIC("kernel not found!");

    // Flush caches (some archs don't like code in their D-cache).
    flush_cache();

    bochs_console_print_str("Launching kernel...\n");
    launch_kernel(entry);

    PANIC("Kernel launch failed!");
}
