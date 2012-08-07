#include "gatekeeper_v1_interface.h"
#include "gatekeeper_v1_impl.h"
#include "gatekeeper_factory_v1_interface.h"
#include "gatekeeper_factory_v1_impl.h"
#include "heap_v1_impl.h"
#include "default_console.h"
#include "exceptions.h"
#include "heap_new.h"
#include "ia32.h" // for NULL_PDID @todo Make this an arch independent definition...

//=====================================================================================================================
// Gatekeeper internal data structures.
//=====================================================================================================================

struct gatekeeper_v1::state_t
{
};

/**
 * State record for reference-counted heap.
 */
struct gatekeeper_heap_state_t
{
    // Used by simple and real gatekeepers
    heap_v1::closure_t  closure;
    bool                destroyable;
    heap_v1::closure_t* heap;
    // mutex mu;

    // Used by real gatekeeper
};

/**
 * State record for simple gatekeeper.
 */
struct simple_gatekeeper_state_t : gatekeeper_v1::state_t
{
    gatekeeper_v1::closure_t  closure;
    heap_v1::closure_t*       heap;        // Heap to return
    heap_v1::closure_t*       alloc_heap;  // Heap state comes from
    stretch_v1::closure_t*    stretch;     // Stretch used for heap
    protection_domain_v1::id  pdid;
    gatekeeper_heap_state_t   heap_state;  // Simple heap state wrapper
};

//=====================================================================================================================
// Simple gatekeeper.
//=====================================================================================================================

/**
 * Get a heap readable and/or writable by the given protection domain. 
 * In this implementation (simple) we have exactly one heap, and hence
 * cannot handle certain options. 
 */
heap_v1::closure_t*
simple_get_heap(gatekeeper_v1::closure_t* self, protection_domain_v1::id pdid, stretch_v1::size size, stretch_v1::rights rights, bool cache)
{
    simple_gatekeeper_state_t* state = reinterpret_cast<simple_gatekeeper_state_t*>(self->d_state);

    if (!cache)
    {
        // Cannot create a new heap; we only have the one. So die.
        OS_RAISE((exception_support_v1::id)"gatekeeper_v1.failure", 0);
    }

    if ((state->pdid != NULL_PDID) && (state->pdid != pdid))
    {
        // Cannot create a new heap for pdom "pdid". So die.
        OS_RAISE((exception_support_v1::id)"gatekeeper_v1.failure", 0);
    }

    if (rights != stretch_v1::rights(stretch_v1::right_read))
    {
        // Cannot chmod the heap either; its read only. So die. 
        OS_RAISE((exception_support_v1::id)"gatekeeper_v1.failure", 0);
    }

    return &state->heap_state.closure;
}

stretch_v1::closure_t*
simple_get_stretch(gatekeeper_v1::closure_t*, protection_domain_v1::id, stretch_v1::size, stretch_v1::rights, uint32_t, uint32_t)
{
    kconsole << "Attempt to call get_stretch() on a simple gatekeeper!" << endl;
    OS_RAISE((exception_support_v1::id)"gatekeeper_v1.failure", 0);
    return 0;
}

static gatekeeper_v1::ops_t simple_gatekeeper_ops =
{
    simple_get_heap,
    simple_get_stretch
};

//=====================================================================================================================
// Gatekeeper heap.
//=====================================================================================================================

static heap_v1::ops_t gatekeeper_heap_ops =
{
    NULL,
    NULL,
    NULL
};

//=====================================================================================================================
// Gatekeeper factory.
//=====================================================================================================================

/**
 * Create a new, general Gatekeeper.
 */
gatekeeper_v1::closure_t*
create(gatekeeper_factory_v1::closure_t* self, stretch_allocator_v1::closure_t* sa, heap_factory_v1::closure_t* hf, heap_v1::closure_t* heap, frame_allocator_v1::closure_t* frames)
{
    return 0;
}

/**
 * Return a gatekeeper for the specified pdom using the stretch provided.
 */
gatekeeper_v1::closure_t*
create_private(gatekeeper_factory_v1::closure_t* self, stretch_v1::closure_t* s, protection_domain_v1::id pdid, heap_factory_v1::closure_t* hf, heap_v1::closure_t* heap)
{
    return 0;
}

/**
 * Return a gatekeeper to run in a single stretch.
 */
gatekeeper_v1::closure_t*
create_global(gatekeeper_factory_v1::closure_t* self, stretch_v1::closure_t* s, heap_factory_v1::closure_t* hf, heap_v1::closure_t* heap)
{
    return 0;
}

/**
 * Return a trivial gatekeeper.
 */
gatekeeper_v1::closure_t*
create_simple(gatekeeper_factory_v1::closure_t* self, heap_v1::closure_t* heap)
{
    simple_gatekeeper_state_t* state = new(heap) simple_gatekeeper_state_t;
    if (!state)
    {
        OS_RAISE((exception_support_v1::id)"heap_v1.no_memory", 0);
    }

    state->heap = heap;
    state->alloc_heap = heap;
    state->stretch = nullptr;
    state->pdid = NULL_PDID; // Use this to mean global.

    closure_init(&state->closure, &simple_gatekeeper_ops, state);

    closure_init(&state->heap_state.closure, &gatekeeper_heap_ops, reinterpret_cast<heap_v1::state_t*>(&state->heap_state));
    state->heap_state.heap = heap;
    state->heap_state.destroyable = false;
    // MU_INIT(&st->heap_st.mu);

    return &state->closure;
}

static gatekeeper_factory_v1::ops_t methods = 
{
    create,
    create_private,
    create_global,
    create_simple
};

static gatekeeper_factory_v1::closure_t clos =
{
    &methods,
    nullptr
};

EXPORT_CLOSURE_TO_ROOTDOM(gatekeeper_factory, v1, clos);
