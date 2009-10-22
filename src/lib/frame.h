#pragma once

#include "types.h"
#include "frame_allocator.h"

/*!
 * Frame abstracts away 4Kb physical frame allocations. It uses a frame_allocator
 * to do actual allocations. This allocator can be replaced at runtime, providing
 * necessary flexibility for e.g. kickstart initialization.
 */
class frame_t
{
public:
    frame_t() {}

    /*!
     * Allocate a frame, identity map it and return @em physical address.
     * @p size argument is ignored. Allocations are always of PAGE_SIZE bytes.
     */
    void* operator new(size_t size);

    /*!
     * Allocate a frame, enter into page mapping at address @p virt
     * and return @em physical address.
     * @p size argument is ignored. Allocations are always of PAGE_SIZE bytes.
     */
    void* operator new(size_t size, address_t virt);

    /*!
     * Delete a frame given @em physical address.
     */
    void operator delete(void* ptr);

    static void set_frame_allocator(frame_allocator_t* alloc) { frame_allocator = alloc; }

private:
    // Copying or assigning a frame is not allowed
    frame_t& operator=(const frame_t&);
    frame_t(const frame_t&);

    static frame_allocator_t* frame_allocator;
};
