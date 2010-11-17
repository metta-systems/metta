#include "frames_module_interface.h"
#include "frames_module_impl.h"

unsigned int frames_module_v1_closure::required_size()
{
    return methods->required_size(this);
}

system_frame_allocator_v1_closure* frames_module_v1_closure::create(int args)
{
    return methods->create(this, args);
}

void frames_module_v1_closure::finish_init()
{
    methods->finish_init(this);
}
