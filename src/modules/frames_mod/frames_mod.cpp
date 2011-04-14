#include "frames_module_interface.h"
#include "frames_module_impl.h"
#include "types.h"
#include "macros.h"
#include "default_console.h"

unsigned int required_size(frames_module_v1_closure* self)
{
    UNUSED(self);
    return 42;
}

static system_frame_allocator_v1_closure* create(frames_module_v1_closure * self, int args)
{
	kconsole << "frames_mod create " << args << endl;
	return 0;
}

static const frames_module_v1_ops ops = {
    required_size,
    create,
    0
};

static const frames_module_v1_closure clos = {
    &ops,
    NULL
};

EXPORT_CLOSURE_TO_ROOTDOM(frames_module_v1, frames_module, clos);
