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

// x86 definition of return address in current frame.
// #define RA() ((address_t)__builtin_return_address(0))

static void 
internal_raise(bool initial_raise, exception_support_v1::closure_t* self, exception_support_v1::id i, exception_support_v1::args a, const char* filename, uint32_t lineno, const char* funcname)
{
	xcp_context_t* ctx;
	xcp_context_t** handlers = reinterpret_cast<xcp_context_t**>(&self->d_state);

	// TRC (word_t *wargs = (word_t *) args;
	// eprintf ("ExnSetjmpMod_Raise: vpp=%x ra=%x exc=%s args=%x\n",
	// 	 Pvs(vp), RA(), id, args);
	// if (args) 
	// eprintf ("  args: %x %x %x %x\n",
	// 	 wargs[0], wargs[1], wargs[2], wargs[3]);            
	// )

	ctx = *handlers;

	// #ifdef __arm
	// #  define r_ra reg[r_pc]
	// #endif
	// TRC (
	// for (; ctx; ctx = ctx->up)
	//   eprintf ("ExnSetjmpMod_Raise: --> ctx=%x ra=%x\n",
	// 	   ctx, ((jmp_buf_t *) &ctx->jmp[0])->r_ra);
	// ctx = *handlers;
	// )
	// #ifdef __arm
	// #  undef r_ra
	// #endif

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
			// FREE( ctx->exn_args );
			// ctx->exn_args = BADPTR;
		}
		ctx = ctx->up;
	}

	if (!ctx)
	{
		PANIC("unhandled exception");
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

	kconsole << "raise: longjmp to context " << ctx << endl;

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
	/*!
	 * We only need one pointer of state, which points to the first
	 * context in the current chain. Thus we use the self->d_state pointer directly.
	 */
	xcp_context_t** handlers = reinterpret_cast<xcp_context_t**>(&self->d_state);

	kconsole << "__ exception_support_setjmp_v1::push_context " << ctx << " handlers " << handlers << endl;

	ctx->state = xcp_none;
	ctx->up = *handlers;
	ctx->down = (xcp_context_t*)0x1; // non-NULL to mark as writable - see the write below!
	ctx->args = NULL;

	/* only write the "down" pointer if we're not the top and the bottom hasn't been cauterised by a RAISE or FINALLY */
	if (*handlers && (*handlers)->down)
	  (*handlers)->down = ctx;
	*handlers = ctx;
}

static void 
exception_support_setjmp_v1_pop_context(exception_support_setjmp_v1::closure_t* self, exception_support_setjmp_v1::context ctx, const char* filename, uint32_t lineno, const char* funcname)
{
	kconsole << "__ exception_support_setjmp_v1::pop_context" << endl;	
}

static exception_support_v1::args 
exception_support_setjmp_v1_allocate_args(exception_support_setjmp_v1::closure_t* self, memory_v1::size size)
{
	kconsole << "__ exception_support_setjmp_v1::allocate_args" << endl;
	return 0;
}

static const exception_support_setjmp_v1::ops_t exception_support_setjmp_v1_methods =
{
	exception_support_setjmp_v1_raise,
	exception_support_setjmp_v1_push_context,
	exception_support_setjmp_v1_pop_context,
	exception_support_setjmp_v1_allocate_args
};

static exception_support_setjmp_v1::closure_t* 
exception_system_v1_create(exception_system_v1::closure_t* self)
{
	kconsole << " ** Exception system - create" << endl;

	exception_support_setjmp_v1::closure_t* cl = new(PVS(heap)) exception_support_setjmp_v1::closure_t;
	if (!cl)
	{
		kconsole << " + FAILED to get memory for exception system." << endl;
		return 0; // Not much point in raising an exception here.
	}

	closure_init(cl, &exception_support_setjmp_v1_methods, reinterpret_cast<exception_support_setjmp_v1::state_t*>(0));
	return cl;
}

static const exception_system_v1::ops_t exception_system_v1_methods =
{
	exception_system_v1_create
};

static const exception_system_v1::closure_t clos =
{
    &exception_system_v1_methods,
    NULL
};

EXPORT_CLOSURE_TO_ROOTDOM(exception_system, v1, clos);
