//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2010, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "macros.h"

/* Privileged */
void sc_activate(vcpu_t*) NORETURN;

/* Unprivileged */
void sc_kernel() NORETURN;
void sc_return() NORETURN; /* Enable activations, reactivate if pending events, else return to caller */
void sc_return_resume(vcpu_context_t*) NORETURN; /* Enable activations, reactivate if pending events, else restore context */

struct vcpu_methods
{
	void (*sc_activate)(vcpu_t*);
	void (*sc_kernel)();
	void (*sc_return)();
	void (*sc_return_resume)(vcpu_context_t*);
};

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
