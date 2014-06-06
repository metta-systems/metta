//
// Part of Metta OS. Check http://atta-metta.net for latest version.
//
// Copyright 2007 - 2012, Stanislav Karchebnyy <berkus@atta-metta.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
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
