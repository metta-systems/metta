#include "gatekeeper_v1_interface.h"
#include "gatekeeper_factory_v1_interface.h"
#include "gatekeeper_factory_v1_impl.h"
#include "heap_new.h"

//=====================================================================================================================
// Gatekeeper internal data structures.
//=====================================================================================================================

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
struct simple_gatekeeper_state_t
{
	gatekeeper_v1::closure_t* closure;
	heap_v1::closure_t*       heap;        // Heap to return
	heap_v1::closure_t*       alloc_heap;  // Heap state comes from
	stretch_v1::closure_t*    stretch;     // Stretch used for heap
	protection_domain_v1::id  pdid;
	gatekeeper_heap_state_t   heap_state;  // Simple heap state wrapper
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
		OS_RAISE("heap_v1.no_memory", 0);
	}

	state->heap = heap;
	state->alloc_heap = heap;
	state->stretch = nullptr;
	state->pdid = NULL_PDID; // Use this to mean global.

	closure_init(&state->closure, &simple_gatekeeper_ops, state);

	closure_init(&state->heap_state.closure, &gatekeeper_heap_ops, &state->heap_state);
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
