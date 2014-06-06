//
// Minimal operator new/delete implementation.
//
// Part of Metta OS. Check http://atta-metta.net for latest version.
//
// Copyright 2007 - 2012, Stanislav Karchebnyy <berkus@atta-metta.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include <new>
#include "macros.h"
#include "panic.h"

void* operator new(size_t size) throw()
{
    PANIC("Default new called!");
    return 0;
}

void operator delete(void*) throw()
{
    PANIC("Default delete called!");
}
