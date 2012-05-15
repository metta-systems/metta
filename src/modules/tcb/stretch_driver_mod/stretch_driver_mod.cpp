//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2012, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "stretch_driver_module_v1_interface.h"
#include "stretch_driver_module_v1_impl.h"
#include "stretch_driver_v1_interface.h"
#include "stretch_driver_v1_impl.h"
#include "stretch_table_v1_interface.h"
#include "stretch_v1_interface.h"
#include "default_console.h"
#include "heap_new.h"
#include "nucleus.h"
#include "ia32.h"

//======================================================================================================================
// stretch_driver_module_v1 methods
// NULL implementation
//======================================================================================================================

struct stretch_driver_v1::state_t
{
    // Ma, look, no state!
};

/**
 * Simply contains a bunch of minimal fields.
 */
struct null_driver_state_t : public stretch_driver_v1::state_t
{
    stretch_driver_v1::closure_t  closure;
    stretch_driver_v1::kind       kind;
    vcpu_v1::closure_t*           vcpu;
    bool                          mode;
    heap_v1::closure_t*           heap;
    stretch_table_v1::closure_t*  stretch_table;
    fault_handler_v1::closure_t*  overrides[memory_v1::fault_max_fault_number];
};

void null_bind(stretch_driver_v1::closure_t* self, stretch_v1::closure_t* stretch, uint32_t page_width)
{
    null_driver_state_t* state = reinterpret_cast<null_driver_state_t*>(self->d_state);

    if (page_width < PAGE_WIDTH)
    {
        kconsole << __FUNCTION__ << ": warning - rounded page_width up to " << PAGE_WIDTH << endl;
        page_width = PAGE_WIDTH;
    }
    else if (page_width > PAGE_WIDTH)
    {
        // Check size is a multiple of the page width.
        memory_v1::address base;
        memory_v1::size size;
        base = stretch->info(&size);
        UNUSED(base);
        if (size & ((1UL << page_width) - 1))
        {
            kconsole << "Not properly aligned size " << size << endl;
            nucleus::debug_stop();
        }
    }

    if (state->stretch_table->put(stretch, page_width, self))
    {
        kconsole << "Major confusion!" << endl;
        nucleus::debug_stop();
    }
}

void null_unbind(stretch_driver_v1::closure_t* self, stretch_v1::closure_t* stretch)
{
    null_driver_state_t* state = reinterpret_cast<null_driver_state_t*>(self->d_state);
    stretch_driver_v1::closure_t* driver;
    uint32_t page_width;
    state->stretch_table->remove(stretch, &page_width, &driver);
}

stretch_driver_v1::kind null_get_kind(stretch_driver_v1::closure_t* self)
{
    null_driver_state_t* state = reinterpret_cast<null_driver_state_t*>(self->d_state);
    return state->kind;
}

stretch_table_v1::closure_t* null_get_table(stretch_driver_v1::closure_t* self)
{
    null_driver_state_t* state = reinterpret_cast<null_driver_state_t*>(self->d_state);
    return state->stretch_table;
}

stretch_driver_v1::result null_map(stretch_driver_v1::closure_t* self, stretch_v1::closure_t* stretch, memory_v1::address virt)
{
    kconsole << __FUNCTION__ << ": mapping not supported!" << endl;
    nucleus::debug_stop();
    return stretch_driver_v1::result_success;
}

stretch_driver_v1::result null_fault(stretch_driver_v1::closure_t* self, stretch_v1::closure_t* stretch, memory_v1::address virt, memory_v1::fault reason)
{
    kconsole << __FUNCTION__ << ": fault handling not supported!" << endl;
    kconsole << __FUNCTION__ << ": fault reason " << reason << endl;
    nucleus::debug_stop();
    return stretch_driver_v1::result_success;
}

fault_handler_v1::closure_t* null_add_handler(stretch_driver_v1::closure_t* self, memory_v1::fault reason, fault_handler_v1::closure_t* handler)
{
    if (reason > memory_v1::fault_max_fault_number)
    {
        kconsole << __FUNCTION__ << ": bogus reason, ignored." << endl;
        return NULL;
    }
    null_driver_state_t* state = reinterpret_cast<null_driver_state_t*>(self->d_state);
    auto result = state->overrides[reason];
    state->overrides[reason] = handler;
    return result;
}

stretch_driver_v1::result null_lock(stretch_driver_v1::closure_t* self, stretch_v1::closure_t* stretch, memory_v1::address virt)
{
    kconsole << __FUNCTION__ << ": locking not supported!" << endl;
    return stretch_driver_v1::result_failure;
}

stretch_driver_v1::result null_unlock(stretch_driver_v1::closure_t* self, stretch_v1::closure_t* stretch, memory_v1::address virt)
{
    kconsole << __FUNCTION__ << ": unlocking not supported!" << endl;
    return stretch_driver_v1::result_failure;
}

memory_v1::size null_revoke_frames(stretch_driver_v1::closure_t* self, memory_v1::size max_frames)
{
    kconsole << __FUNCTION__ << ": revoking frames not supported!" << endl;
    return 0;
}

static const stretch_driver_v1::ops_t stretch_driver_v1_null_methods =
{
    null_bind,
    null_unbind,
    null_get_kind,
    null_get_table,
    null_map,
    null_fault,
    null_add_handler,
    null_lock,
    null_unlock,
    null_revoke_frames,
};

//======================================================================================================================
// stretch_driver_module_v1 methods
//======================================================================================================================

/*
 * create_null: create an (essentially) empty stretch driver. It can perform no mapping or fault handling,
 * but simply produces a closure of the correct type during system startup.
 * Not useful for standard user-domains.
 */
static stretch_driver_v1::closure_t* create_null(stretch_driver_module_v1::closure_t* self, heap_v1::closure_t* heap, stretch_table_v1::closure_t* strtab)
{
    kconsole << __PRETTY_FUNCTION__ << endl;
    auto state = new(heap) null_driver_state_t;

    if (!state)
        return NULL;

    state->kind = stretch_driver_v1::kind_null;
    state->vcpu = NULL; // don't have/need
    state->heap = heap;
    state->stretch_table = strtab;

    for(size_t i = 0; i < memory_v1::fault_max_fault_number; ++i)
        state->overrides[i] = NULL;

    closure_init(&state->closure, &stretch_driver_v1_null_methods, state);

    return &state->closure;
}

static const stretch_driver_module_v1::ops_t stretch_driver_module_v1_methods =
{
    create_null,
    NULL,
    NULL,
    NULL
};

static const stretch_driver_module_v1::closure_t clos =
{
    &stretch_driver_module_v1_methods,
    NULL
};

EXPORT_CLOSURE_TO_ROOTDOM(stretch_driver_module, v1, clos);
