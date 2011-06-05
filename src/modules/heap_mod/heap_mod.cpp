#include "heap_module_v1_interface.h"
#include "heap_module_v1_impl.h"
#include "heap_v1_interface.h"
#include "heap_v1_impl.h"
#include "heap.h"
#include "memory.h"
#include "new.h"
#include "default_console.h"

//======================================================================================================================
// heap_v1 implementation
//======================================================================================================================

struct heap_v1_state
{
    heap_v1_closure closure;
    heap_t* heap;
};

static memory_v1_address heap_v1_allocate(heap_v1_closure* self, memory_v1_size size)
{
    lockable_scope_lock_t lock(*self->state->heap);
    return reinterpret_cast<memory_v1_address>(self->state->heap->allocate(size));
}

static void heap_v1_free(heap_v1_closure* self, memory_v1_address ptr)
{
    lockable_scope_lock_t lock(*self->state->heap);
    self->state->heap->free(reinterpret_cast<void*>(ptr));
}

static const heap_v1_ops heap_v1_methods = {
    heap_v1_allocate,
    heap_v1_free
};

//======================================================================================================================
// heap_module_v1 implementation
//======================================================================================================================

static heap_v1_closure* heap_module_v1_create_raw(heap_module_v1_closure* self, memory_v1_address where, memory_v1_size size)
{
    kconsole << "heap_mod: create_raw at " << where << " with " << int(size) << " bytes." << endl;

    size = page_align_up(size);
    if (size < HEAP_MIN_SIZE - sizeof(heap_v1_state))
    {
        kconsole << "Too small heap requested, not allocating!" << endl;
        return 0;
    }

    heap_v1_state* state = reinterpret_cast<heap_v1_state*>(where);

    heap_v1_closure* ret = &state->closure;
    ret->methods = &heap_v1_methods;
    ret->state = state;

    address_t start = where + sizeof(heap_v1_state) + sizeof(heap_t);
    state->heap = new(reinterpret_cast<void*>(where + sizeof(heap_v1_state))) heap_t(start, start + size);

    return ret;
}

static memory_v1_address heap_module_v1_where(heap_module_v1_closure* self, heap_v1_closure* heap, memory_v1_size* size)
{
    return 0;
}

static const heap_module_v1_ops ops = {
    heap_module_v1_create_raw,
    heap_module_v1_where
};

static const heap_module_v1_closure clos = {
    &ops,
    NULL
};

EXPORT_CLOSURE_TO_ROOTDOM(heap_module_v1, heap_module, clos);
