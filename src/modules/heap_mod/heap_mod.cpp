//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2011, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
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

static void heap_v1_check(heap_v1_closure* self, bool /*check_free_blocks*/)
{
    lockable_scope_lock_t lock(*self->state->heap);
    self->state->heap->check_integrity();
}

static const heap_v1_ops heap_v1_methods = {
    heap_v1_allocate,
    heap_v1_free,
    heap_v1_check
};

//======================================================================================================================
// heap_module_v1 implementation
//======================================================================================================================

static heap_v1_closure* heap_module_v1_create_raw(heap_module_v1_closure* self, memory_v1_address where, memory_v1_size size)
{
    kconsole << __FUNCTION__ << ": at " << where << " with " << int(size) << " bytes." << endl;

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

    address_t end = where + size;
    address_t start = where + sizeof(heap_v1_state) + sizeof(heap_t);
    state->heap = new(reinterpret_cast<void*>(where + sizeof(heap_v1_state))) heap_t(start, end);

    return ret;
}

static memory_v1_address heap_module_v1_where(heap_module_v1_closure* self, heap_v1_closure* heap, memory_v1_size* size)
{
    return 0;
}

/*!
 * Realize is used to turn a 'raw' heap into a stretch-based one, and requires that the given stretch maps exactly over
 * the frames of the original heap.
 */
static heap_v1_closure* heap_module_v1_realize(heap_module_v1_closure* self, heap_v1_closure* raw_heap, stretch_v1_closure* stretch)
{
    // switch heap type to 'stretch'
    // clear out all stretches
    // map given stretch as a single stretch
    // replace ops with stretch based ones
    return raw_heap;
}

static const heap_module_v1_ops ops = {
    heap_module_v1_create_raw,
    heap_module_v1_where,
    heap_module_v1_realize
};

static const heap_module_v1_closure clos = {
    &ops,
    NULL
};

EXPORT_CLOSURE_TO_ROOTDOM(heap_module, v1, clos);

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
