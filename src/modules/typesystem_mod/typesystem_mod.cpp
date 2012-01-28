#include "type_system_f_v1_interface.h"
#include "type_system_f_v1_impl.h"
#include "type_system_factory_v1_interface.h"
#include "type_system_factory_v1_impl.h"

static type_system_f_v1::closure_t* create(type_system_factory_v1::closure_t* self, heap_v1::closure_t* h, map_card64_address_v1::closure_t* cardmap, map_string_address_v1::closure_t* stringmap)
{
	return 0;
}

static type_system_factory_v1::ops_t methods = 
{
	create
};

static type_system_factory_v1::closure_t clos =
{
	&methods,
	NULL
};

EXPORT_CLOSURE_TO_ROOTDOM(type_system_factory, v1, clos);
