//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2011, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
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
void launch_kernel(address_t entry);// NORETURN;
void flush_cache();

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
