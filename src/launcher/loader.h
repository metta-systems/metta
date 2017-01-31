//
// Part of Metta OS. Check https://atta-metta.net for latest version.
//
// Copyright 2007 - 2017, Stanislav Karchebnyy <berkus@atta-metta.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "types.h"
#include "macros.h"

/**
 * A particular type of loader format (e.g. multiboot-compliant loader).
 */
class loader_format_t
{
public:
    /**
     * String describing current loader format.
     */
    const char* name;

    /**
     * Detect if a valid loader format of this particular type is present.
     * @returns true if format found, false otherwise.
     */
    bool (*probe)(void);

    /**
     * Initialize everything according to loader format.
     * This function will parse sysimage, install nucleus, load necessary pre-boot environment 
     * and return an entry point address to jump to for continuing the boot sequence.
     * @returns entry point for boot sequence.
     */
    address_t (*prepare)(void);
};

#define NULL_LOADER { "null", NULL, NULL }

/**
 * NULL_LOADER-terminated array of loader formats.
 */
extern loader_format_t loader_formats[];

// Prototypes for architecture-specific functions
void launch_kernel(address_t entry);// NORETURN;
void flush_cache();
