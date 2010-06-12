#include "frame_allocator_interface.h"
#include "frame_allocator_impl.h"

client_frame_allocator_closure* frame_allocator_closure::new_client()
{
    return methods->new_client(this);
}
