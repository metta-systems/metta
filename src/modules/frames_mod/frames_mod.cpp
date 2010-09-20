#include "frames_module_interface.h"
#include "frames_module_impl.h"
#include "types.h"
#include "macros.h"

unsigned int required_size(frames_module_v1_closure* self)
{
    UNUSED(self);
    return 42;
}

static const frames_module_v1_ops ops = {
    required_size,
    0,
    0
};

static const frames_module_v1_closure clos = {
    &ops,
    NULL
};

EXPORT_CL_TO_ROOTDOM(frames_module_v1, frames_module, clos);
