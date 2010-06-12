#pragma once

#include "module_interface.h"

// frames_module is a construction interface for frames module, it creates and returns an instance of (singleton)
// frame_allocator type.
// (metatype? metaclass? factory?)

struct frame_allocator_closure;

DECLARE_CLOSURE(frames_module)
{
    void required(int args);
    frame_allocator_closure* create(int args);
    void done();
};
