#include "nemesis/exception_support_setjmp_v1_interface.h"
#include "nemesis/exception_support_setjmp_v1_impl.h"
#include "nemesis/exception_system_v1_interface.h"
#include "nemesis/exception_system_v1_impl.h"
#include <setjmp.h>

jmp_buf b;
/*static exception_system_v1_create()
{
	
}

static internal_raise()
{
	
}

static exception_support_setjmp_v1_raise()
{
	
}

static exception_support_setjmp_v1_push_context()
{
	
}

static exception_support_setjmp_v1_pop_context()
{
	
}

static exception_support_setjmp_v1_allocate_args()
{
	
}

static const exception_system_v1::ops_t exception_system_v1_methods =
{
	exception_system_v1_create
};

static const exception_support_setjmp_v1::ops_t exception_support_setjmp_v1_methods =
{
	exception_support_setjmp_v1_raise,
	exception_support_setjmp_v1_push_context,
	exception_support_setjmp_v1_pop_context,
	exception_support_setjmp_v1_allocate_args
};

static const exception_system_v1::closure_t clos =
{
    &exception_system_v1_methods,
    NULL
};

EXPORT_CLOSURE_TO_ROOTDOM(exceptions_module, v1, clos);*/
