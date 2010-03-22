#include "loader.h"
#include "panic.h"
#include "debugger.h"

/*!
 * Main kickstart loader function. Parses through all loader formats to find a valid one.
 */
extern "C" void loader()
{
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

    bochs_console_print_str("format->init()\n");
    address_t entry = format->init();

    // Flush caches (some archs don't like code in their D-cache).
    flush_cache();

    bochs_console_print_str("Launching kernel...\n");
    launch_kernel(entry);

    PANIC("Kernel launch failed!");
}
