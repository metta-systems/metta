#pragma once

struct frame_allocator_ops
{
    void (*allocate)(frame_allocator_closure* self);
    void (*allocate_range)(frame_allocator_closure* self);
    void (*query)(frame_allocator_closure* self);
    void (*free)(frame_allocator_closure* self);
    void (*destroy)(frame_allocator_closure* self);
};
