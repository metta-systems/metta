#pragma once

struct frames_module_closure;
struct frame_allocator_closure;

// ops structure should be exposed to module implementors!
struct frames_module_ops
{
    void                     (*required)(frames_module_closure* self, int args);
    frame_allocator_closure* (*create)(frames_module_closure* self, int args);
    void                     (*done)(frames_module_closure* self);
};
