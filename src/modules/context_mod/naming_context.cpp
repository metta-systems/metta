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
#include "exceptions.h"
#include "panic.h"
#include <unordered_map>
#include "heap_allocator.h"

// required:
// types.any implementation
// sequence<> meddler support

// steps:
// implement types.any and type_system
// implement naming_context

using namespace std;

typedef const char* key_type;
typedef types::any value_type;
typedef pair<key_type, value_type> pair_type;
typedef unordered_map<key_type, value_type, hash<key_type>, equal_to<key_type>, heap_allocator<pair_type>> context_map;

struct naming_context_v1::state_t
{
	context_map map;
	heap_v1::closure_t* heap;
	naming_context_v1::closure_t closure;

	state_t(heap_allocator<pair_type>* alloc, heap_v1::closure_t* heap_) : map(*alloc), heap(heap_) {}
};

static naming_context_v1::names
naming_context_v1_list(naming_context_v1::closure_t* self)
{
	naming_context_v1::names n;
	for (auto x : self->d_state->map)
	{
		// n.push_back(x.first);
		kconsole << "Returning naming_context key " << x.first << endl;
	}

	return n;
}

// This is incomplete, doesn't support compound arc-names.
static bool
naming_context_v1_get(naming_context_v1::closure_t *self, const char *key, types::any *out_value)
{
    // card64table_t::iterator it = self->d_state->table->find(k);
    // if (it != self->d_state->table->end())
    // {
    // 	*v = (*it).second;
    //     return true;
    // }
    // return false;

	return false;//self->d_state->map.TryGet(key, *out_value);
}

// This is incomplete, doesn't support compound arc-names.
static void
naming_context_v1_add(naming_context_v1::closure_t *self, const char *key, types::any value)
{
	// return self->d_state->table->insert(std::make_pair(k, v)).second;
	// self->d_state->map.Put(key, value);
}

// This is incomplete, doesn't support compound arc-names.
static void
naming_context_v1_remove(naming_context_v1::closure_t *self, const char *key)
{
	// self->d_state->map.Remove(key);
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

	heap_allocator<pair_type>* alloc = new heap_allocator<pair_type>(heap);
	naming_context_v1::state_t* state = new(heap) naming_context_v1::state_t(alloc, heap);

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
