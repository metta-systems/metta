#include "heap_module_v1_interface.h"
#include "heap_module_v1_impl.h"
#include "types.h"
#include "macros.h"
#include "default_console.h"
#include "bootinfo.h"
#include "algorithm"

static heap_v1_closure* heap_module_v1_create_raw(heap_module_v1_closure* self, memory_v1_address where, memory_v1_size size)
{
    kconsole << "heap_mod: create_raw at " << where << " with " << int(size) << " bytes." << endl;
    return 0;
}

static memory_v1_address heap_module_v1_where(heap_module_v1_closure* self, heap_v1_closure* heap, memory_v1_size* size)
{
    return 0;
}

static const heap_module_v1_ops ops = {
    heap_module_v1_create_raw,
    heap_module_v1_where
};

static const heap_module_v1_closure clos = {
    &ops,
    NULL
};

EXPORT_CLOSURE_TO_ROOTDOM(heap_module_v1, heap_module, clos);
