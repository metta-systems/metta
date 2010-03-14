//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2010, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "component_interface.h"

// Userspace interface to stretch allocator.
interface stretch_allocator_v1 : public component_interface_t
{
    /*!
     * Allocate a stretch of given size with given access rights for current domain.
     * @param[in] size Size in bytes of the stretch. Will be rounded up to the nearest supported page size.
     * @param[in] access Access specifiers for the stretch.
     * @param[in] base Base address to locate stretch at. Value 0 means any available address.
     * @returns Newly allocated stretch, which can then be bound to a stretch driver, or NULL in case of error.
     */
    method const stretch_t* allocate_stretch(size_t size, stretch_t::access_t access, address_t base = 0);
    /*!
     * Release a stretch owned by current domain.
     * Physical frames and virtual address mappings will be invalidated and reclaimed by the system.
     * @param[in] stretch Stretch to release.
     */
    method void release_stretch(stretch_t* stretch);
};

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
