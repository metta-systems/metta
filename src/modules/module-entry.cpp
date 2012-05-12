//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2012, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "panic.h"

//======================================================================================================================
// Dummy entry point for components.
// Will panic because entry point is not meant to be called, components are entered through an interface closure.
// Defined as weak, because root_domain component defines it's own _start atm - this is to be changed.
//======================================================================================================================

extern "C" void _start()
{
    PANIC("Do not call module entry directly!");
}
