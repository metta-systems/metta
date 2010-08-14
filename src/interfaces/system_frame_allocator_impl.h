#pragma once

struct client_frame_allocator_ops
{
    void (*allocate)(client_frame_allocator_closure* self);
    void (*allocate_range)(client_frame_allocator_closure* self);
    void (*query)(client_frame_allocator_closure* self);
    void (*free)(client_frame_allocator_closure* self);
    void (*destroy)(client_frame_allocator_closure* self);
};
