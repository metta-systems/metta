#include "vcpu.h"

static vcpu_t* kernel_vcpu;
static vcpu_t* active_vcpu;

static vcpu_context_t* active_context;

/* Shared page */
#define SHARED_PAGE attribute((section(".shared_page")))

uint64_t current_time SHARED_PAGE;
uint64_t alarm_time SHARED_PAGE;

// Address of this syscall table is passed in VCPU interface to processes
vcpu_methods syscall_table SHARED_PAGE;

// This is glue activation handler for startup.
// This is the very first code executed at the beginning of bootstrap.
// 
void glue_init()
{
	syscall_table.sc_activate      = &sc_activate;
	syscall_table.sc_kernel        = &sc_kernel;
	syscall_table.sc_return        = &sc_return;
	syscall_table.sc_return_resume = &sc_return_resume;
}

void sc_activate(vcpu_t* new_vcpu) NORETURN
{
}

void sc_kernel() NORETURN
{
}

void sc_return() NORETURN
{
}

void sc_return_resume(vcpu_context_t*) NORETURN
{
}
