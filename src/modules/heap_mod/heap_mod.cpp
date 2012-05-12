//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2012, Stanislav Karchebnyy <berkus@exquance.com>
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
#include "exceptions.h"
#include "panic.h"

//======================================================================================================================
// heap_v1 implementation
//======================================================================================================================

struct heap_v1::state_t
{
    heap_v1::closure_t closure;
    heap_t* heap;
};

static memory_v1::address heap_v1_allocate(heap_v1::closure_t* self, memory_v1::size size)
{
#if !SMP
    ASSERT(!self->d_state->heap->has_lock());
#endif
    lockable_scope_lock_t lock(*self->d_state->heap);
    void* res = 0;

    // This mega-ugly is here because we behave differently before and after the exceptions module is instantiated...
    if (PVS(exceptions))
    {
        OS_TRY {
            res = self->d_state->heap->allocate(size);
        }
        OS_FINALLY {
            lock.unlock();
        }
        OS_ENDTRY

        if (!res)
            OS_RAISE((exception_support_v1::id)"heap_v1.no_memory", NULL);
    }
    else
    {
        // Cannot RAISE here at all!
        res = self->d_state->heap->allocate(size);
    }

    return reinterpret_cast<memory_v1::address>(res);
}

static void heap_v1_free(heap_v1::closure_t* self, memory_v1::address ptr)
{
#if !SMP
    ASSERT(!self->d_state->heap->has_lock());
#endif
    lockable_scope_lock_t lock(*self->d_state->heap);
    self->d_state->heap->free(reinterpret_cast<void*>(ptr));
}

static void heap_v1_check(heap_v1::closure_t* self, bool /*check_free_blocks*/)
{
    lockable_scope_lock_t lock(*self->d_state->heap);
    self->d_state->heap->check_integrity();
}

static const heap_v1::ops_t heap_v1_methods =
{
    heap_v1_allocate,
    heap_v1_free,
    heap_v1_check
};

//======================================================================================================================
// heap_module_v1 implementation
//======================================================================================================================

static heap_v1::closure_t* heap_module_v1_create_raw(heap_module_v1::closure_t* self, memory_v1::address where, memory_v1::size size)
{
    kconsole << __FUNCTION__ << ": at " << where << " with " << int(size) << " bytes." << endl;

    size = page_align_up(size);
    if (size < HEAP_MIN_SIZE - sizeof(heap_v1::state_t))
    {
        kconsole << __FUNCTION__ << ": too small heap requested, not allocating!" << endl;
        return 0;
    }

    heap_v1::state_t* state = reinterpret_cast<heap_v1::state_t*>(where);

    heap_v1::closure_t* ret = &state->closure;
    closure_init(ret, &heap_v1_methods, state);

    address_t end = where + size;
    address_t start = where + sizeof(heap_v1::state_t) + sizeof(heap_t);
    // TODO: heap could be constructed as a member of state_t?
    state->heap = new(reinterpret_cast<void*>(where + sizeof(heap_v1::state_t))) heap_t(start, end);

    return ret;
}

static memory_v1::address heap_module_v1_where(heap_module_v1::closure_t* self, heap_v1::closure_t* heap, memory_v1::size* size)
{
    return 0;
}

/*!
 * Realize is used to turn a 'raw' heap into a stretch-based one, and requires that the given stretch maps exactly over
 * the frames of the original heap.
 */
static heap_v1::closure_t* heap_module_v1_realize(heap_module_v1::closure_t* self, heap_v1::closure_t* raw_heap, stretch_v1::closure_t* stretch)
{
    // switch heap type to 'stretch'
    // clear out all stretches
    // map given stretch as a single stretch
    // replace ops with stretch based ones
    return raw_heap;
}

static const heap_module_v1::ops_t heap_module_v1_methods =
{
    heap_module_v1_create_raw,
    heap_module_v1_where,
    heap_module_v1_realize
};

static const heap_module_v1::closure_t clos =
{
    &heap_module_v1_methods,
    NULL
};

EXPORT_CLOSURE_TO_ROOTDOM(heap_module, v1, clos);
