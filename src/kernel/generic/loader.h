#pragma once

#include "types.h"
#include "macros.h"

/*!
 * A particular type of loader format (e.g. multiboot-compliant loader).
 */
class loader_format_t
{
public:
    /*!
     * String describing current loader format.
     */
    const char* name;

    /*!
     * Detect if a valid loader format of this particular type is present.
     * @returns true if format found, false otherwise.
     */
    bool (*probe)(void);

    /*!
     * Initialize everything according to loader format.
     * @returns entry point for kernel.
     */
    address_t (*init)(void);
};

#define NULL_LOADER { "null", NULL, NULL }

/*!
 * NULL_LOADER-terminated array of loader formats.
 */
extern loader_format_t loader_formats[];

// Prototypes for architecture-specific functions
void launch_kernel(address_t entry) NORETURN;
void flush_cache();
