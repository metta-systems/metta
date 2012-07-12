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
#include "type_system_v1_interface.h"
#include "default_console.h"
#include "module_interface.h"
#include "exceptions.h"
#include "panic.h"
#include <unordered_map>
#include "heap_new.h"
#include "heap_allocator.h"
#include "stringref.h"
#include "stringstuff.h"

// required:
// sequence<> meddler support - std::vector<T> for now, but looking into using sequence_t<T> wrapper instead

using namespace std;

typedef const char* key_type;
typedef types::any value_type;
typedef pair<key_type, value_type> pair_type;
typedef heap_allocator<pair_type> context_allocator;
typedef unordered_map<key_type, value_type, hash<key_type>, equal_to<key_type>, context_allocator> context_map;

struct naming_context_v1::state_t
{
	naming_context_v1::closure_t closure;
	context_map map;
	heap_v1::closure_t* heap;
	type_system_v1::closure_t* typesystem;

	state_t(context_allocator* alloc, heap_v1::closure_t* heap_) : map(*alloc), heap(heap_) {}
};

static naming_context_v1::names
list(naming_context_v1::closure_t* self)
{
	naming_context_v1::names n(context_allocator(self->d_state->heap));
	for (auto x : self->d_state->map)
	{
		n.push_back(x.first);
	}

	return n;
}

/**
 * Look up a name in the context.
 */
static bool
get(naming_context_v1::closure_t *self, const char *key, types::any *out_value)
{
	naming_context_v1::state_t* state = self->d_state;

    stringref_t name_sr(key);
    // Split key into first component and the rest.
    std::pair<stringref_t, stringref_t> refs = name_sr.split('.');

    // @todo: move this allocation to stringref_t guts?
    if (!refs.second.empty())
        key = string_n_copy(refs.first.data(), refs.first.size(), PVS(heap)); // @todo MEMLEAK

    context_map::iterator it = state->map.find(key);
	// There was only one component and so we can get the value and return.
	if (refs.second.empty())
	{
	    if (it != state->map.end())
	    {
	    	*out_value = (*it).second;
	        return true;
	    }
	    return false;
	}
	else
	{
		// At this stage there is another component. Thus we need to check
		// that the types.any actually is a subtype of naming_context, and then
		// recurse.
		
		// Check conformance with the context type 
	    if (it != state->map.end())
	    {
	    	if (state->typesystem->is_type(out_value->type_, naming_context_v1::type_code))
	    	{
	    		naming_context_v1::closure_t* nctx = reinterpret_cast<naming_context_v1::closure_t*>(state->typesystem->narrow(*out_value, naming_context_v1::type_code));
	    		// @todo support either passing stringrefs or extracting cstrings out of stringrefs...
	    		return nctx->get(refs.second.data(), out_value);
	    	}
		    else
		    {
				// Have to check for exceptions presence, since get is caled before exception system is set up.
				if(PVS(exceptions)) {
					OS_RAISE((exception_support_v1::id)"naming_context_v1.not_context", 0);
				} else {
					kconsole << __FUNCTION__ << ": not a context " << (*it).first << endl;
				    return false;
				}
		    }
		}
		// Haven't found this item
		kconsole << __FUNCTION__ << ": failed to go deeper." << endl;
		return false;
	}
}

// This is incomplete, doesn't support compound arc-names.
/**
 * Bind a name in the context (not necessarily this one!).
 */
static void
add(naming_context_v1::closure_t *self, const char *key, types::any value)
{
	self->d_state->map.insert(make_pair(key, value));
	// return self->d_state->table->insert(std::make_pair(k, v)).second;
}

// This is incomplete, doesn't support compound arc-names.
static void
remove(naming_context_v1::closure_t *self, const char *key)
{
    context_map::iterator it = self->d_state->map.find(key);
    if (it != self->d_state->map.end())
    {
        self->d_state->map.erase(it);
        // return true;
    }
    // return false;
}

static void
destroy(naming_context_v1::closure_t* self)
{
    self->d_state->map.clear();
}

static const naming_context_v1::ops_t naming_context_v1_methods =
{
	list,
	get,
	add,
	remove,
	destroy
};

//=====================================================================================================================
// The Factory
//=====================================================================================================================

static naming_context_v1::closure_t*
create_context(naming_context_factory_v1::closure_t* self, heap_v1::closure_t* heap, type_system_v1::closure_t* type_system)
{
	kconsole << " ** Creating new naming context." << endl;

	context_allocator* alloc = new(heap) context_allocator(heap);
	naming_context_v1::state_t* state = new(heap) naming_context_v1::state_t(alloc, heap);
	state->typesystem = type_system;

	closure_init(&state->closure, &naming_context_v1_methods, state);
	return &state->closure;
}

static const naming_context_factory_v1::ops_t naming_context_factory_v1_methods =
{
	create_context
};

static const naming_context_factory_v1::closure_t clos =
{
    &naming_context_factory_v1_methods,
    NULL
};

EXPORT_CLOSURE_TO_ROOTDOM(naming_context_factory, v1, clos);
