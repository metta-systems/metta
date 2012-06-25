#include "naming_context_v1_interface.h"
#include "naming_context_v1_impl.h"
#include "interface_v1_interface.h"
#include "interface_v1_impl.h"
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

static bool
extends(interface_v1::closure_t* self, interface_v1::closure_t** o)
{
	return false;
}

static bool
info(interface_v1::closure_t* self, interface_v1::needs* need_list, types::name* name, types::code* code)
{
	return false;
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

interface_v1::ops_t interface_ops = {
	list,
	get,
	shared_add,
	shared_remove,
	shared_destroy,
	extends,
	info
};
