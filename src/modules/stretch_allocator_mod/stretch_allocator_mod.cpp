#include "system_stretch_allocator_v1_interface.h"
#include "system_stretch_allocator_v1_impl.h"

static const system_stretch_allocator_v1_closure clos = {
    NULL,
    NULL
};

EXPORT_CLOSURE_TO_ROOTDOM(system_stretch_allocator, v1, clos);
