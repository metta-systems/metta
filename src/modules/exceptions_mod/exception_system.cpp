#include "nemesis/exception_support_setjmp_v1_interface.h"
#include "nemesis/exception_support_setjmp_v1_impl.h"
#include "nemesis/exception_system_v1_interface.h"
#include "nemesis/exception_system_v1_impl.h"
#include "heap_v1_interface.h"
#include "infopage.h"
#include "default_console.h"
#include "module_interface.h"
#include <setjmp.h>

/*static internal_raise()
{
	
}*/

static void 
exception_support_setjmp_v1_raise(exception_support_setjmp_v1::closure_t* self, exception_support_v1::id i, exception_support_v1::args a, const char* filename, uint32_t lineno, const char* funcname)
{
	
}

static void 
exception_support_setjmp_v1_push_context(exception_support_setjmp_v1::closure_t* self, exception_support_setjmp_v1::context ctx)
{
	
}

static void 
exception_support_setjmp_v1_pop_context(exception_support_setjmp_v1::closure_t* self, exception_support_setjmp_v1::context ctx, const char* filename, uint32_t lineno, const char* funcname)
{
	
}

static exception_support_v1::args 
exception_support_setjmp_v1_allocate_args(exception_support_setjmp_v1::closure_t* self, memory_v1::size size)
{
	return 0;
}

static const exception_support_setjmp_v1::ops_t exception_support_setjmp_v1_methods =
{
	exception_support_setjmp_v1_raise,
	exception_support_setjmp_v1_push_context,
	exception_support_setjmp_v1_pop_context,
	exception_support_setjmp_v1_allocate_args
};

static exception_support_setjmp_v1::closure_t* exception_system_v1_create(exception_system_v1::closure_t* self)
{
	kconsole << " ** Exception system - create" << endl;

	/*!
	 * We only need one pointer of state, which points to the first
	 * context in the current chain. Thus we use the self->st pointer
	 * directly.
	 */

	exception_support_setjmp_v1::closure_t* cl;
	
	if (!(cl = reinterpret_cast<exception_support_setjmp_v1::closure_t*>(PVS(heap)->allocate(sizeof(*cl)))))
	{
		kconsole << " + FAILED to get memory for exception system." << endl;
		return 0; // Not much point in raising an exception here.
	}

	closure_init(cl, &exception_support_setjmp_v1_methods, reinterpret_cast<exception_support_setjmp_v1::state_t*>(0));
	return cl;
}

static const exception_system_v1::ops_t exception_system_v1_methods =
{
	exception_system_v1_create
};

static const exception_system_v1::closure_t clos =
{
    &exception_system_v1_methods,
    NULL
};

EXPORT_CLOSURE_TO_ROOTDOM(exception_system, v1, clos);
