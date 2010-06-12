#pragma once

#include "module_interface.h"

DECLARE_CLOSURE(client_frame_allocator)
{
    void allocate();
    void allocate_range();
    void query();
    void free();
    void destroy();
}
