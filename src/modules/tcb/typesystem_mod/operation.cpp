#include "naming_context_v1_interface.h"
#include "naming_context_v1_impl.h"
#include "interface_v1_interface.h"
#include "operation_v1_interface.h"
#include "operation_v1_impl.h"
#include "exception_v1_impl.h"
#include "interface_v1_state.h"
#include "exceptions.h"

//=====================================================================================================================
// Method implementations.
//=====================================================================================================================

static naming_context_v1::names
list(naming_context_v1::closure_t* self)
{
    return naming_context_v1::names();
}

static bool
get(naming_context_v1::closure_t* self, const char* name, types::any* obj)
{
    return false;
}

static operation_v1::kind
info(operation_v1::closure_t* self, const char** name, interface_v1::closure_t** i, uint32_t* index, uint32_t* a, uint32_t* r, uint32_t* e)
{
	return operation_v1::kind_proc;
}

static exception_v1::list
exceptions(operation_v1::closure_t* self, heap_v1::closure_t* heap)
{
	return exception_v1::list();
}

//=====================================================================================================================
// add, remove, dup and destroy methods from naming_context are all nulls.
//=====================================================================================================================

static void
shared_add(naming_context_v1::closure_t*, const char*, types::any)
{
    OS_RAISE((exception_support_v1::id)"naming_context_v1.denied", 0);
    return;
}

static void
shared_remove(naming_context_v1::closure_t*, const char*)
{ 
    OS_RAISE((exception_support_v1::id)"naming_context_v1.denied", 0);
    return; 
}

// dup is not yet defined, but it raises denied as well

static void
shared_destroy(naming_context_v1::closure_t*)
{ 
    return; 
}

//=====================================================================================================================
// Method suites
//=====================================================================================================================

operation_v1::ops_t operation_ops = {
	list,
	get,
	shared_add,
	shared_remove,
	shared_destroy,
	info,
	exceptions
};
