#pragma once

#include "macros.h"

// i386
#define _JBLEN 6
#define _JBTYPE void*

typedef _JBTYPE jmp_buf[_JBLEN];

// Rename functions so they don't clash with whatever builtins there might be.
// implemented in runtime/setjmp.s
extern "C" int __sjljeh_setjmp(jmp_buf buf);
extern "C" void __sjljeh_longjmp(jmp_buf buf, int retval) NEVER_RETURNS;
