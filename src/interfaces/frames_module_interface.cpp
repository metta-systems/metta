#include "frames_module_interface.h"
#include "frames_module_impl.h"

void frames_module_v1_closure::required(int args)
{
    methods->required(this, args);
}

system_frame_allocator_v1_closure* frames_module_v1_closure::create(int args)
{
    return methods->create(this, args);
}

void frames_module_v1_closure::done()
{
    methods->done(this);
}
