#include "frames_module_interface.h"
#include "frames_module_impl.h"

void frames_module_closure::required(int args)
{
    methods->required(this, args);
}

frame_allocator_closure* frames_module_closure::create(int args)
{
    return methods->create(this, args);
}

void frames_module_closure::done()
{
    methods->done(this);
}
