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

struct stretch_driver_v1_state {};

/*!
 * Simply contains a bunch of minimal fields.
 */
struct null_driver_state_t : public stretch_driver_v1_state
{
    stretch_driver_v1_closure closure;
    stretch_driver_v1_kind kind;
    vcpu_v1_closure*      vcpu;
    bool                  mode;
    heap_v1_closure*      heap;
    stretch_table_v1_closure* stretch_table;
    fault_handler_v1_closure* overrides[memory_v1_fault_max_fault_number];
};

void null_bind(stretch_driver_v1_closure* self, stretch_v1_closure* stretch, uint32_t page_width)
{
    null_driver_state_t* state = reinterpret_cast<null_driver_state_t*>(self->state);

    if (page_width < PAGE_WIDTH)
    {
        kconsole << __FUNCTION__ << ": warning - rounded page_width up to " << PAGE_WIDTH << endl;
        page_width = PAGE_WIDTH;
    }
    else if (page_width > PAGE_WIDTH)
    {
        // Check size is a multiple of the page width.
        memory_v1_address base;
        memory_v1_size size;
        base = stretch->info(&size);
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

void null_unbind(stretch_driver_v1_closure* self, stretch_v1_closure* stretch)
{
    null_driver_state_t* state = reinterpret_cast<null_driver_state_t*>(self->state);
    stretch_driver_v1_closure* driver;
    uint32_t page_width;
    state->stretch_table->remove(stretch, &page_width, &driver);
}

stretch_driver_v1_kind null_get_kind(stretch_driver_v1_closure* self)
{
    null_driver_state_t* state = reinterpret_cast<null_driver_state_t*>(self->state);
    return state->kind;
}

stretch_table_v1_closure* null_get_table(stretch_driver_v1_closure* self)
{
    null_driver_state_t* state = reinterpret_cast<null_driver_state_t*>(self->state);
    return state->stretch_table;
}

stretch_driver_v1_result null_map(stretch_driver_v1_closure* self, stretch_v1_closure* stretch, memory_v1_address virt)
{
    kconsole << __FUNCTION__ << ": mapping not supported!" << endl;
    nucleus::debug_stop();
    return stretch_driver_v1_result_success;
}

stretch_driver_v1_result null_fault(stretch_driver_v1_closure* self, stretch_v1_closure* stretch, memory_v1_address virt, memory_v1_fault reason)
{
    kconsole << __FUNCTION__ << ": fault handling not supported!" << endl;
    kconsole << __FUNCTION__ << ": fault reason " << reason << endl;
    nucleus::debug_stop();
    return stretch_driver_v1_result_success;
}

fault_handler_v1_closure* null_add_handler(stretch_driver_v1_closure* self, memory_v1_fault reason, fault_handler_v1_closure* handler)
{
    if (reason > memory_v1_fault_max_fault_number)
    {
        kconsole << __FUNCTION__ << ": bogus reason, ignored." << endl;
        return NULL;
    }
    null_driver_state_t* state = reinterpret_cast<null_driver_state_t*>(self->state);
    auto result = state->overrides[reason];
    state->overrides[reason] = handler;
    return result;
}

stretch_driver_v1_result null_lock(stretch_driver_v1_closure* self, stretch_v1_closure* stretch, memory_v1_address virt)
{
    kconsole << __FUNCTION__ << ": locking not supported!" << endl;
    return stretch_driver_v1_result_failure;
}

stretch_driver_v1_result null_unlock(stretch_driver_v1_closure* self, stretch_v1_closure* stretch, memory_v1_address virt)
{
    kconsole << __FUNCTION__ << ": unlocking not supported!" << endl;
    return stretch_driver_v1_result_failure;
}

memory_v1_size null_revoke_frames(stretch_driver_v1_closure* self, memory_v1_size max_frames)
{
    kconsole << __FUNCTION__ << ": revoking frames not supported!" << endl;
    return 0;
}

static const stretch_driver_v1_ops stretch_driver_v1_null_methods = {
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
static stretch_driver_v1_closure* create_null(stretch_driver_module_v1_closure* self, heap_v1_closure* heap, stretch_table_v1_closure* strtab)
{
    kconsole << __PRETTY_FUNCTION__ << endl;
    auto state = new(heap) null_driver_state_t;

    if (!state)
        return NULL;

    state->kind = stretch_driver_v1_kind_null;
    state->vcpu = NULL; // don't have/need
    state->heap = heap;
    state->stretch_table = strtab;

    for(size_t i = 0; i < memory_v1_fault_max_fault_number; ++i)
        state->overrides[i] = NULL;

    state->closure.methods = &stretch_driver_v1_null_methods;
    state->closure.state = state;

    return &state->closure;
}

static const stretch_driver_module_v1_ops stretch_driver_module_v1_methods = {
    create_null,
    NULL,
    NULL,
    NULL
};

static const stretch_driver_module_v1_closure clos = {
    &stretch_driver_module_v1_methods,
    NULL
};

EXPORT_CLOSURE_TO_ROOTDOM(stretch_driver_module, v1, clos);
