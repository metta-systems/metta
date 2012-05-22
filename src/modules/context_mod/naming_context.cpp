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
#include <zstl/include/ZHashMap.hpp>

// required:
// types.any implementation
// sequence<> meddler support

// steps:
// implement types.any and type_system
// implement naming_context

typedef ZHashMap<const char*, types::any> context_map;
// typedef std::unordered_map<const char*, types::any*> context_map;

struct naming_context_v1::state_t
{
	context_map map;
	heap_v1::closure_t* heap;
	naming_context_v1::closure_t closure;
};

static naming_context_v1::names
naming_context_v1_list(naming_context_v1::closure_t* self)
{
	ZList<const char*> keys = self->d_state->map.Keys();
	return naming_context_v1::names();
}

// This is incomplete, doesn't support compound arc-names.
static bool
naming_context_v1_get(naming_context_v1::closure_t *self, const char *key, types::any *out_value)
{
	return self->d_state->map.TryGet(key, *out_value);
}

// This is incomplete, doesn't support compound arc-names.
static void
naming_context_v1_add(naming_context_v1::closure_t *self, const char *key, types::any value)
{
	self->d_state->map.Put(key, value);
}

// This is incomplete, doesn't support compound arc-names.
static void
naming_context_v1_remove(naming_context_v1::closure_t *self, const char *key)
{
	self->d_state->map.Remove(key);
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

	naming_context_v1::state_t* state = new(heap) naming_context_v1::state_t;
	state->heap = heap;

	closure_init(&state->closure, &naming_context_v1_methods, state);
	return &state->closure;
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
