//
// Minimal operator new/delete implementation.
//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2010, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "new.h"
#include "macros.h"
#include "memory.h"
#include "memutils.h"
#include "default_console.h"
#include "debugger.h"

void* operator new[](size_t size)
{
    return operator new(size);
}

void* operator new(size_t size)
{
    PANIC("Default new called!");
}

void operator delete(void*)
{
    PANIC("Default delete called!");
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
