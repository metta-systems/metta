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
