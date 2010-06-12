#pragma once

struct frame_allocator_ops : public client_frame_allocator_ops
{
    client_frame_allocator_closure* (*new_client)(frame_allocator_closure* self);
};
