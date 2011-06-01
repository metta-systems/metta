#include "frames_module_v1_interface.h"
#include "frames_module_v1_impl.h"
#include "types.h"
#include "macros.h"
#include "default_console.h"
#include "bootinfo.h"

static memory_v1_size frames_module_v1_required_size(frames_module_v1_closure* self)
{
    UNUSED(self);
    return 42;
}

//create(physmem allmem, physmem used

static system_frame_allocator_v1_closure* frames_module_v1_create(frames_module_v1_closure* self, ramtab_v1_closure* rtab, memory_v1_address where)
{
    UNUSED(self);
    bootinfo_t* bi = new(BOOTINFO_PAGE) bootinfo_t(false); // simplify memory map operations
    
    
    
	kconsole << "frames_mod create @ " << where << endl;
	return 0;
}

static void frames_module_v1_finish_init(frames_module_v1_closure* self, system_frame_allocator_v1_closure* frames, heap_v1_closure* heap)
{
    
}

static const frames_module_v1_ops ops = {
    frames_module_v1_required_size,
    frames_module_v1_create,
    frames_module_v1_finish_init
};

static const frames_module_v1_closure clos = {
    &ops,
    NULL
};

EXPORT_CLOSURE_TO_ROOTDOM(frames_module_v1, frames_module, clos);
