//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2011, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "loader.h"
#include "panic.h"
#include "default_console.h"

/*!
 * Main kickstart loader function. Parses through all loader formats to find a valid one.
 */
extern "C" void loader()
{
    kconsole << "loader()\n";
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

    kconsole << format->name << " init()\n";
    address_t entry = format->init();

    if (!entry)
        PANIC("kernel not found!");

    // Flush caches (some archs don't like code in their D-cache).
    flush_cache();

    kconsole << "Launching kernel at " << entry << endl;
    launch_kernel(entry);

    PANIC("Kernel launch failed!");
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
