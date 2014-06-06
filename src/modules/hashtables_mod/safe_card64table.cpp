//
// Part of Metta OS. Check http://atta-metta.net for latest version.
//
// Copyright 2007 - 2012, Stanislav Karchebnyy <berkus@atta-metta.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
/**
 * Implement safe_card64table and card64table modules as well as factories for them.
 *
 * It is a simple wrapper around STL map class.
 * TODO: Since this type of wrapper is often repeated (see stretch_table_mod, string_address_table)
 * it makes sense to make a generic reusable version.
 */
#include "map_card64_address_factory_v1_interface.h"
#include "map_card64_address_factory_v1_impl.h"
#include "map_card64_address_v1_interface.h"
#include "map_card64_address_v1_impl.h"
#include "heap_v1_interface.h"
#include "default_console.h"
#include "hashtables.h"
#include "heap_new.h"

using namespace std;

DECLARE_MAP(card64table, map_card64_address_v1::key, map_card64_address_v1::value);

struct map_card64_address_v1::state_t
{
    map_card64_address_v1::closure_t closure;
    heap_v1::closure_t* heap;
    card64table_t* table;
};

static bool get(map_card64_address_v1::closure_t* self, map_card64_address_v1::key k, map_card64_address_v1::value* v)
{
    card64table_t::iterator it = self->d_state->table->find(k);
    if (it != self->d_state->table->end())
    {
        *v = (*it).second;
        return true;
    }
    return false;
}

static bool put(map_card64_address_v1::closure_t* self, map_card64_address_v1::key k, map_card64_address_v1::value v)
{
    return self->d_state->table->insert(std::make_pair(k, v)).second;
}

static bool remove(map_card64_address_v1::closure_t* self, map_card64_address_v1::key k, map_card64_address_v1::value* v)
{
    card64table_t::iterator it = self->d_state->table->find(k);
    if (it != self->d_state->table->end())
    {
        *v = (*it).second;
        self->d_state->table->erase(it);
        return true;
    }
    return false;
}

static uint32_t size(map_card64_address_v1::closure_t* self)
{
    return self->d_state->table->size();
}

static void dispose(map_card64_address_v1::closure_t* self) // RENAME to destroy()? See stretch_table_mod for ref.
{
    self->d_state->table->clear();
    //TODO: delete self
}

static struct map_card64_address_v1::ops_t map_methods =
{
    get,
    put,
    remove,
    size,
    dispose
};

//=====================================================================================================================
// The Factory
//=====================================================================================================================

static map_card64_address_v1::closure_t*
map_card64_address_factory_v1_create(map_card64_address_factory_v1::closure_t* self, heap_v1::closure_t* heap)
{
    map_card64_address_v1::state_t* state = new(heap) map_card64_address_v1::state_t;
    auto heap_alloc = new(heap) card64table_heap_allocator(heap); // FIXME: a mem leak!
    // TODO: if (!state) raise Exception -- heap will raise no_memory itself!
    state->heap = heap;
    state->table = new(heap) card64table_t(*heap_alloc);
    closure_init(&state->closure, &map_methods, state);
    return &state->closure;
}

static struct map_card64_address_factory_v1::ops_t methods =
{
    map_card64_address_factory_v1_create
};

static struct map_card64_address_factory_v1::closure_t clos =
{
    &methods,
    nullptr
};

EXPORT_CLOSURE_TO_ROOTDOM(map_card64_address_factory, v1, clos);

