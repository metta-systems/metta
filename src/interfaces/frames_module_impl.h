#pragma once

struct frames_module_v1_closure;
struct system_frame_allocator_v1_closure;

// ops structure should be exposed to module implementors!
struct frames_module_v1_ops
{
    unsigned int                       (*required_size)(frames_module_v1_closure* self);
    system_frame_allocator_v1_closure* (*create)(frames_module_v1_closure* self, int args);
    void                               (*finish_init)(frames_module_v1_closure* self);
};
