#include "frames_module_impl.h"
#include "macros.h"

unsigned int required_size(frames_module_v1_closure* self)
{
    UNUSED(self);
    return 42;
}

frames_module_v1_ops ops = {
    required_size,
    0,
    0
};
