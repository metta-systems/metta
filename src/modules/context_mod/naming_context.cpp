//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2012, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "naming_context_v1_interface.h"
#include "naming_context_v1_impl.h"
#include "naming_context_factory_v1_interface.h"
#include "naming_context_factory_v1_impl.h"
#include "default_console.h"
#include "module_interface.h"
#include "heap_new.h"
#include "exceptions.h"
#include "panic.h"
#include <unordered_map>

// required:
// types.any implementation
// sequence<> meddler support

// steps:
// implement types.any and type_system
// implement naming_context

typedef std::unordered_map<const char*, types::any*> context_map;

static naming_context_v1::names
naming_context_v1_list(naming_context_v1::closure_t* self)
{
	// return self->state->map.keys();
	return naming_context_v1::names();
}

static bool
naming_context_v1_get(naming_context_v1::closure_t *, const char *, types::any *)
{
	return false;
}

static void
naming_context_v1_add(naming_context_v1::closure_t *, const char *, types::any)
{

}

static void
naming_context_v1_remove(naming_context_v1::closure_t *, const char *)
{

}

static void
naming_context_v1_destroy(naming_context_v1::closure_t *)
{

}

static const naming_context_v1::ops_t naming_context_v1_methods =
{
	naming_context_v1_list,
	naming_context_v1_get,
	naming_context_v1_add,
	naming_context_v1_remove,
	naming_context_v1_destroy
};

//=====================================================================================================================
// The Factory
//=====================================================================================================================

static naming_context_v1::closure_t*
naming_context_factory_v1_create_context(naming_context_factory_v1::closure_t* self, heap_v1::closure_t* heap, type_system_v1::closure_t* type_system)
{
	kconsole << " ** Creating new naming context." << endl;

	naming_context_v1::closure_t* cl = new(PVS(heap)) naming_context_v1::closure_t;
	if (!cl)
	{
		// raise heap no_memory;
	}

	closure_init(cl, &naming_context_v1_methods, reinterpret_cast<naming_context_v1::state_t*>(0));
	return cl;
}

static const naming_context_factory_v1::ops_t naming_context_factory_v1_methods =
{
	naming_context_factory_v1_create_context
};

static const naming_context_factory_v1::closure_t clos =
{
    &naming_context_factory_v1_methods,
    NULL
};

EXPORT_CLOSURE_TO_ROOTDOM(naming_context_factory, v1, clos);
