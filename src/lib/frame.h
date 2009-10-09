#pragma once

#include "types.h"

/*!
 * Frame abstracts away 4Kb physical frame allocations.
 */
class frame_t
{
public:
    frame_t() {}

    /*!
     * Allocate a frame, enter into page mapping at address @p virt
     * and return @em physical address.
     * @p size argument is ignored.
     */
    void* operator new(size_t size, address_t& virt);

    /*!
     * Delete a frame given @em physical address.
     */
    void operator delete(void* ptr);

private:
    // Copying or assigning a frame is not allowed
    frame_t& operator=(const frame_t&);
    frame_t(const frame_t&);
};
