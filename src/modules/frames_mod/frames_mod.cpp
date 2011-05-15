#include "frames_module_v1_interface.h"
#include "frames_module_v1_impl.h"
#include "types.h"
#include "macros.h"

static unsigned int required_size(frames_module_v1_closure* self)
{
    UNUSED(self);
    return 42;
}

static const frames_module_v1_ops ops = {
    required_size
};

static const frames_module_v1_closure clos = {
    &ops,
    NULL
};

EXPORT_CL_TO_ROOTDOM(frames_module_v1, frames_module, clos);
