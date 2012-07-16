//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2012, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "nemesis/exception_support_setjmp_v1_interface.h"
#include "nemesis/exception_support_setjmp_v1_impl.h"
#include "nemesis/exception_system_v1_interface.h"
#include "nemesis/exception_system_v1_impl.h"
#include "heap_v1_interface.h"
#include "infopage.h"
#include "default_console.h"
#include "module_interface.h"
#include "heap_new.h"
#include "exceptions.h"
#include "panic.h"

//=====================================================================================================================
// setjmp()-based exception system.
//=====================================================================================================================

static void 
internal_raise(bool initial_raise, exception_support_v1::closure_t* self, exception_support_v1::id i, exception_support_v1::args a, const char* filename, uint32_t lineno, const char* funcname) NEVER_RETURNS
{
    xcp_context_t* ctx;
    xcp_context_t** handlers = reinterpret_cast<xcp_context_t**>(&self->d_state);

    ctx = *handlers;

    while (ctx && ctx->state != xcp_none)
    {
        kconsole << "exception stack: state " << ctx->state << ", can't happen?" << endl;
        if (ctx->args)
        {
            /* Another exception was in the middle of being processed when
             * this one happened. We have decided that we must pass higher
             * up the stack than that exception. Thus its argument
             * record is unreachable and must be freed.
             */
            // FREE( ctx->args );
            // ctx->args = BADPTR;
        }
        ctx = ctx->up;
    }

    if (!ctx)
    {
        kconsole << "Unhandled exception " << (const char*)i << " raised from " << filename << ":" << (int)lineno << " (in function " << funcname << ")" << endl;
        PANIC("unhandled exception system abort");
        /* TODO: abort domain; threads' top-level fn should have a handler */
    }

    ctx->state    = xcp_active;
    ctx->name     = (const char*)i;
    ctx->args     = a;
    ctx->filename = filename;
    ctx->line     = lineno;
    ctx->funcname = funcname;
    if (initial_raise)
        ctx->down = NULL; // cauterise base of the backtrace

    /* we've already longjmp'ed to this xcp context, so pop it */
    *handlers = ctx->up;

    D(kconsole << "raise: longjmp to context " << ctx << endl);

    D(kconsole << "raise: jmp_buf words" << endl);
    D(for (size_t x = 0; x < _JBLEN; ++x)
        kconsole << "word " << x << ": " << ctx->jmp[x] << endl);

    xcp_longjmp (ctx->jmp, 1);
}

static void 
exception_support_setjmp_v1_raise(exception_support_v1::closure_t* self, exception_support_v1::id i, exception_support_v1::args a, const char* filename, uint32_t lineno, const char* funcname)
{
    kconsole << "__ exception_support_setjmp_v1::raise" << endl;
    internal_raise(true, self, i, a, filename, lineno, funcname);
}

static void 
exception_support_setjmp_v1_push_context(exception_support_setjmp_v1::closure_t* self, exception_support_setjmp_v1::context c)
{
    xcp_context_t* ctx = reinterpret_cast<xcp_context_t*>(c);
    /*
     * We only need one pointer of state, which points to the first
     * context in the current chain. Thus we use the self->d_state pointer directly.
     */
    xcp_context_t** handlers = reinterpret_cast<xcp_context_t**>(&self->d_state);

    V(kconsole << "__ exception_support_setjmp_v1::push_context " << ctx << " handlers " << handlers << endl);

    ctx->state = xcp_none;
    ctx->up = *handlers;
    ctx->down = (xcp_context_t*)0x1; // non-NULL to mark as writable - see the write below!
    ctx->args = 0;

    /* only write the "down" pointer if we're not the top and the bottom hasn't been cauterised by a RAISE or FINALLY */
    if (*handlers && (*handlers)->down)
      (*handlers)->down = ctx;
    *handlers = ctx;
}

/* precondition: ctx.state = none or active */
static void 
exception_support_setjmp_v1_pop_context(exception_support_setjmp_v1::closure_t* self, exception_support_setjmp_v1::context c, const char* filename, uint32_t lineno, const char* funcname)
{
    xcp_context_t* ctx = reinterpret_cast<xcp_context_t*>(c);
    xcp_context_t** handlers = reinterpret_cast<xcp_context_t**>(&self->d_state);
    xcp_state_t prev_state = ctx->state;

    V(kconsole << "__ exception_support_setjmp_v1::pop_context " << ctx << ", prev_state " << prev_state << endl);

    /* set state to popped so that OS_FINALLY only pops once in normal case */
    ctx->state = xcp_popped;

    /* pop the context */
    *handlers = ctx->up;

    if (prev_state == xcp_active)
    {
        /* Exception was active, so propagate it up. */
        internal_raise(false, reinterpret_cast<exception_support_v1::closure_t*>(self), (exception_support_v1::id)ctx->name, ctx->args, filename, lineno, funcname);
        /* NOTREACHED */
    }
    else if (ctx->args)
    {
        // FREE( ctx->args );
        // ctx->args = BADPTR;
    }
}

static exception_support_v1::args 
exception_support_setjmp_v1_allocate_args(exception_support_setjmp_v1::closure_t* self, memory_v1::size size)
{
    D(kconsole << "__ exception_support_setjmp_v1::allocate_args " << size << endl);
    address_t res = PVS(heap)->allocate(size);

    // if (!res)
    // {
    //  OS_RAISE((exception_support_v1::id)"heap_v1.no_memory", NULL); // hmm wait, the heap will throw that itself! no need for this check
    //  PANIC("NOTREACHED");
    // }

    return res;
}

static const exception_support_setjmp_v1::ops_t exception_support_setjmp_v1_methods =
{
    exception_support_setjmp_v1_raise,
    exception_support_setjmp_v1_push_context,
    exception_support_setjmp_v1_pop_context,
    exception_support_setjmp_v1_allocate_args
};

//=====================================================================================================================
// The Factory
//=====================================================================================================================

static exception_support_setjmp_v1::closure_t* 
exception_system_v1_create(exception_system_v1::closure_t* self)
{
    kconsole << " ** Exception system - create" << endl;

    exception_support_setjmp_v1::closure_t* cl = new(PVS(heap)) exception_support_setjmp_v1::closure_t;
    if (!cl)
    {
        kconsole << " + FAILED to get memory for exception system. This is quite fatal." << endl;
        return 0; // Not much point in raising an exception here.
    }

    closure_init(cl, &exception_support_setjmp_v1_methods, reinterpret_cast<exception_support_setjmp_v1::state_t*>(0));
    return cl;
}

static const exception_system_v1::ops_t exception_system_v1_methods =
{
    exception_system_v1_create
};

static exception_system_v1::closure_t clos =
{
    &exception_system_v1_methods,
    NULL
};

EXPORT_CLOSURE_TO_ROOTDOM(exception_system, v1, clos);
