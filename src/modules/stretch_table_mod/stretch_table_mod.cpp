#include "stretch_table_module_v1_interface.h"
#include "stretch_table_module_v1_impl.h"

static const stretch_table_module_v1_closure clos = {
    NULL,
    NULL
};

EXPORT_CLOSURE_TO_ROOTDOM(stretch_table_module, v1, clos);
