#include "frame_allocator_interface.h"
#include "frame_allocator_impl.h"

void frame_allocator_closure::allocate()
{
    methods->allocate(this);
}

void frame_allocator_closure::allocate_range()
{
    methods->allocate_range(this);
}

void frame_allocator_closure::query()
{
    methods->query(this);
}

void frame_allocator_closure::free()
{
    methods->free(this);
}

void frame_allocator_closure::destroy()
{
    methods->destroy(this);
}
