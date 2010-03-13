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
