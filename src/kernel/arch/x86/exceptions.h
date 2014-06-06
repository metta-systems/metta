//
// Metta exceptions support. Quite primitive for now. Mostly borrowed from Nemesis.
// C++ exceptions must be disabled in the kernel and modules for this to work reliably.
//
// Part of Metta OS. Check http://atta-metta.net for latest version.
//
// Copyright 2007 - 2012, Stanislav Karchebnyy <berkus@atta-metta.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include <algorithm> // for std::min
#include "setjmp.h"
#include "infopage.h"
#include "registers.h"
#include "nemesis/exception_support_setjmp_v1_interface.h"

/**
 * Exception handling state in the current OS_TRY clause.
 *
 * The state is "none" when no exception has been raised, "active" when
 * one has been raised but has not yet been caught by a CATCH clause, and
 * "handled" after the exception has been caught by some CATCH clause.
 * "popped" for state already taken off the stack.
 */
enum xcp_state_t
{
	xcp_none = 0,
	xcp_active = 1,
	xcp_handled = 2,
	xcp_popped = 3
};

/**
 * Exception handling context record.
 *
 * A context block is allocated in the current stack frame for each
 * OS_TRY clause.  These context blocks are linked to form a stack of
 * all current OS_TRY blocks in the current thread.  Each context block
 * contains a jump buffer for use by setjmp and longjmp.
 */
struct xcp_context_t
{
	jmp_buf        jmp;      // Jump buffer.
	xcp_context_t *up;       // Link up to the next context block in stack.
	xcp_context_t *down;     // Link down to the previous context block in stack (zero at the bottom).
	xcp_state_t    state;    // State of handling for this OS_TRY.
	const char*    name;     // Name of the current exception.
	address_t      args;     // Exception arguments record address
	const char*    filename; // Which file raised it
	const char*    funcname; // Which function raised it
	uint32_t       line;     // In which line
};

// Exception binding helper defines.
#define xcp_push_context(ctx)  PVS(exceptions)->push_context(ctx)
#define xcp_pop_context(ctx)   PVS(exceptions)->pop_context(ctx, __FILE__, __LINE__, __FUNCTION__)
#define xcp_rec_alloc(size)    PVS(exceptions)->allocate_args(size)
#define xcp_setjmp(buf)        (__sjljeh_setjmp(buf))
#define xcp_longjmp(buf, j)    (__sjljeh_longjmp(buf, j))

/**
 * Define "routine" to determine if two exceptions match.
 *
 * Effectively just strcmp(), but can't use the stack (at least, not much).
 */
#define xcp_matches(e1,e2) \
({						\
    const char *s=e1, *d=e2;				\
    while(*s == *d && *s != 0 && *d != 0)	\
    {						\
	s++;					\
	d++;					\
    }						\
    (*s == 0 && *d == 0)? 1 : 0;		\
})

// Exception handling macros.
// Use these when writing server code.
#define OS_RAISE(e, args) PVS(exceptions)->raise(e, args, __FILE__, __LINE__, __FUNCTION__)
#define OS_RERAISE        PVS(exceptions)->raise(__xcp_ctx.name, __xcp_ctx.args, __FILE__, __LINE__, __FUNCTION__)

/**
 * Start a new TRY block, which may contain exception handlers
 *
 *   Allocate a context block on the stack to remember the current
 *   exception. Push it on the context block stack.  Initialize
 *   this context block to indicate that no exception is active. Do a SETJMP
 *   to snapshot this environment (or return to it).  Then, start
 *   a block of statements to be guarded by the TRY clause.
 *   This block will be ended by one of the following: a CATCH, CATCH_ALL,
 *   or the ENDTRY macros.
 */
#define OS_TRY \
	{ \
		xcp_context_t __xcp_ctx; \
		xcp_context_t* __xcp_save; \
		address_t __xcp_base; \
		xcp_push_context(&__xcp_ctx); \
		if (!xcp_setjmp(__xcp_ctx.jmp)) \
		{
			/* user's code goes here */

/**
 * Define a CATCH(e) clause (or exception handler).
 *
 *   First, end the prior block.  Then, check if the current exception
 *   matches what the user is trying to catch with the CATCH clause.
 *   If there is a match, a variable is declared to support lexical
 *   nesting of RERAISE statements, and the state of the current
 *   exception is changed to "handled".
 */
#define OS_CATCH(e) \
		} \
		else if (xcp_matches(__xcp_ctx.name, (e))) { \
			__xcp_ctx.state = xcp_handled;
			/* user's code goes here */

/**
 * Define a CATCH_ALL clause (or "catchall" handler).
 *
 *   First, end the prior block.  Then, unconditionally,
 *   let execution enter into the catchall code.  As with a normal
 *   catch, the state of the current exception is changed to "handled".
 */
#define OS_CATCH_ALL \
		} \
		else { \
			__xcp_ctx.state = xcp_handled;
			/* user's code goes here */

/**
 * Define a FINALLY clause
 *
 *   This "keyword" starts a FINALLY clause.  It must appear before
 *   an ENDTRY.  A FINALLY clause will be entered after normal exit
 *   of the TRY block, or if an unhandled exception tries to propagate
 *   out of the TRY block.
 *
 *			** WARNING **
 *   You should *avoid* using FINALLY with CATCH clauses, that is, use it
 *   only as TRY {} FINALLY {} ENDTRY.
 */
#define OS_FINALLY \
		} \
		if (__xcp_ctx.state == xcp_none) xcp_pop_context(&__xcp_ctx); \
		if (__xcp_ctx.state == xcp_active && __xcp_ctx.up) __xcp_ctx.up->down = nullptr; /* cauterise */ \
		{   /* this is not part of if, this is just a new code block opened. */
			/* user's code goes here */

/**
 * End the whole TRY clause.
 */
#define OS_ENDTRY \
		} \
		if (__xcp_ctx.state == xcp_active) \
		{ \
			/* preserve trace on stack. assumes stack grows downwards */ \
			__xcp_save = &__xcp_ctx; \
			__xcp_base = address_t(&__xcp_ctx); \
			while (__xcp_save->down) \
			{ \
				__xcp_save = __xcp_save->down; \
				__xcp_base = std::min(__xcp_base, address_t(__xcp_save)); \
			} \
			__xcp_base = read_stack_pointer() - __xcp_base; \
			if (__xcp_base > 0) \
				__builtin_alloca(__xcp_base); \
		} \
		if (__xcp_ctx.state == xcp_none || __xcp_ctx.state == xcp_active) xcp_pop_context(&__xcp_ctx); \
	}

