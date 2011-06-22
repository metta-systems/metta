#include "stretch_driver_module_v1_interface.h"
#include "stretch_driver_module_v1_impl.h"
#include "stretch_driver_v1_interface.h"
#include "default_console.h"

//======================================================================================================================
// stretch_driver_module_v1 methods
//======================================================================================================================

static stretch_driver_v1_closure* create_null(stretch_driver_module_v1_closure* self, heap_v1_closure* heap, stretch_table_v1_closure* strtab)
{
    kconsole << __PRETTY_FUNCTION__ << endl;
    return 0;
}

static const stretch_driver_module_v1_ops stretch_driver_module_v1_methods = {
    create_null,
    NULL,
    NULL,
    NULL
};

static const stretch_driver_module_v1_closure clos = {
    &stretch_driver_module_v1_methods,
    NULL
};

EXPORT_CLOSURE_TO_ROOTDOM(stretch_driver_module, v1, clos);
