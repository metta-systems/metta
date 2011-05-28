#include "frames_module_v1_interface.h"
#include "frames_module_v1_impl.h"
#include "types.h"
#include "macros.h"
#include "default_console.h"

static unsigned int required_size(frames_module_v1_closure* self)
{
    UNUSED(self);
    return 42;
}

static /*system_frame_allocator_v1_closure* */ void create(frames_module_v1_closure * self, uint32_t size)
{
	kconsole << "frames_mod create " << size << endl;
	return ;
}

static const frames_module_v1_ops ops = {
    required_size,
    create
};

static const frames_module_v1_closure clos = {
    &ops,
    NULL
};

EXPORT_CLOSURE_TO_ROOTDOM(frames_module_v1, frames_module, clos);
