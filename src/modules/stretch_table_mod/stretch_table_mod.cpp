//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2011, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "stretch_table_module_v1_interface.h"
#include "stretch_table_module_v1_impl.h"
#include "default_console.h"
#include "heap_new.h"
#include "heap_allocator.h"

//======================================================================================================================
// stretch_table_v1 implementation
//======================================================================================================================

#include "stretch_table_v1_interface.h"
#include "stretch_table_v1_impl.h"
#include "stretch_driver_v1_interface.h"
#include "stretch_v1_interface.h"
#include "heap_v1_interface.h"
#include "hash_map"
#include "functional"

struct hash_fn : std::unary_function<size_t, const stretch_v1::closure_t*>
{
    size_t operator()(const stretch_v1::closure_t* s) const
    {
        address_t w = reinterpret_cast<address_t>(s);
        return w ^ (w >> 18);
    }
};

struct equal_fn : std::binary_function<bool, const stretch_v1::closure_t*, const stretch_v1::closure_t*>
{
    bool operator()(stretch_v1::closure_t* a, stretch_v1::closure_t* b) const
    {
        return a == b;
    }
};

// tuple might be a better choice here?
struct driver_rec
{
    driver_rec(stretch_driver_v1::closure_t* d, size_t p) : driver(d), page_width(p) {}
    stretch_driver_v1::closure_t* driver;
    size_t page_width;
};

typedef std::allocator<std::pair<stretch_v1::closure_t*, driver_rec>> stretch_heap_allocator;
typedef std::hash_map<stretch_v1::closure_t*, driver_rec, hash_fn, equal_fn, stretch_heap_allocator> stretch_map;

struct stretch_table_v1::state_t
{
    stretch_map* stretches;
    heap_v1::closure_t* heap;
};

static bool get(stretch_table_v1::closure_t* self, stretch_v1::closure_t* stretch, uint32_t* page_width, stretch_driver_v1::closure_t** stretch_driver)
{
    stretch_map::iterator it = self->d_state->stretches->find(stretch);
    if (it != self->d_state->stretches->end())
    {
        driver_rec result = (*it).second;
        *page_width = result.page_width;
        *stretch_driver = result.driver;
        return true;
    }
    return false;
}

static bool put(stretch_table_v1::closure_t* self, stretch_v1::closure_t* stretch, uint32_t page_width, stretch_driver_v1::closure_t* stretch_driver)
{
    return self->d_state->stretches->insert(std::make_pair(stretch, driver_rec(stretch_driver, page_width))).second;
}

static bool remove(stretch_table_v1::closure_t* self, stretch_v1::closure_t* stretch, uint32_t* page_width, stretch_driver_v1::closure_t** stretch_driver)
{
    stretch_map::iterator it = self->d_state->stretches->find(stretch);
    if (it != self->d_state->stretches->end())
    {
        driver_rec result = (*it).second;
        *page_width = result.page_width;
        *stretch_driver = result.driver;
        self->d_state->stretches->erase(it);
        return true;
    }
    return false;
}

static void destroy(stretch_table_v1::closure_t* self)
{
    kconsole << "Trying to destroy a stretch_table, might not work!" << endl;
    delete/*(self->state->heap)*/ self->d_state->stretches; //hmmmm, need to use right allocators and stuff..
}

static const stretch_table_v1::ops_t stretch_table_v1_methods =
{
    get,
    put,
    remove,
    destroy
};

//======================================================================================================================
// stretch_table_module_v1 implementation: TODO rename _module to _factory?
//======================================================================================================================

static stretch_table_v1::closure_t* create(stretch_table_module_v1::closure_t* self, heap_v1::closure_t* heap)
{
    stretch_table_v1::state_t* new_state = new(heap) stretch_table_v1::state_t;
    auto heap_alloc = new(heap) std::heap_allocator_implementation(heap);

    new_state->heap = heap;
    new_state->stretches = new(heap) stretch_map(heap_alloc);

    stretch_table_v1::closure_t* cl = new(heap) stretch_table_v1::closure_t;
    closure_init(cl, &stretch_table_v1_methods, new_state);

    return cl;
}

static const stretch_table_module_v1::ops_t stretch_table_module_v1_methods =
{
    create 
};

static const stretch_table_module_v1::closure_t clos =
{
    &stretch_table_module_v1_methods,
    NULL
};

EXPORT_CLOSURE_TO_ROOTDOM(stretch_table_module, v1, clos);

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
