#pragma once

#include "module_interface.h"

// frames_module is a construction interface for frames module, it creates and returns an instance of (singleton)
// frame_allocator type.
// (metatype? metaclass? factory?)

struct system_frame_allocator_v1_closure;

DECLARE_CLOSURE(frames_module_v1)
{
    unsigned int required_size();
    system_frame_allocator_v1_closure* create(int args);
    void finish_init();
};
