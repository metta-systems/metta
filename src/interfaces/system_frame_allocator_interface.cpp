#include "client_frame_allocator_interface.h"
#include "client_frame_allocator_impl.h"

void client_frame_allocator_closure::allocate()
{
    methods->allocate(this);
}

void client_frame_allocator_closure::allocate_range()
{
    methods->allocate_range(this);
}

void client_frame_allocator_closure::query()
{
    methods->query(this);
}

void client_frame_allocator_closure::free()
{
    methods->free(this);
}

void client_frame_allocator_closure::destroy()
{
    methods->destroy(this);
}
