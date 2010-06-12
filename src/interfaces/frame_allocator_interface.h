#pragma once

#include "client_frame_allocator_interface.h"

// frame allocator is a privileged interface that can allocate memory for privileged domain and also create
// client frame allocators for client (userspace) domains.

DECLARE_CLOSURE_(frame_allocator, client_frame_allocator)
{
    client_frame_allocator_closure* new_client();
};
