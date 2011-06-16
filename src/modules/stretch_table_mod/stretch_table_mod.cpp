#include "stretch_table_module_v1_interface.h"
#include "stretch_table_module_v1_impl.h"

static stretch_table_v1_closure* create(stretch_table_module_v1_closure* self, heap_v1_closure* heap)
{
    return 0;
}

static const stretch_table_module_v1_ops ops = {
    create
};

static const stretch_table_module_v1_closure clos = {
    &ops,
    NULL
};

EXPORT_CLOSURE_TO_ROOTDOM(stretch_table_module, v1, clos);
