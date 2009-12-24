//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

/*!
 * Virtual address space in Metta is single, shared between all processes. virtual_address_space_t describes only part
 * of the whole address space, available to a particular process.
 *
 * Each virtual space has @c stretches of addresses of particular type - kernel, user or reserved. Kernel stretches
 * correspond to kernel data and code, common in all address spaces. User type addresses are available to the process.
 * Reserved types are either reserved for memory-mapped I/O or for future use by kernel structures and are unavailable
 * to memory allocation inside process.
 *
 * virtual_address_space_t also wraps around processor's paging mechanism and provides mapping/unmapping facilities
 * for memory pages.
 */
class virtual_address_space_t
{
public:
    virtual ~virtual_address_space_t();
};
