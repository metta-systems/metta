//
// Copyright 2007 - 2008, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Naive C++ wrapper around HP's atomic_ops library.
//
#pragma once

#include "types.h"
#include "atomic/atomic_ops.h"

class atomic_ops
{
public:
    static address_t exchange(address_t *lock, address_t old_val, address_t new_val);
};

// kate: indent-width 4; replace-tabs on;
// vi:set ts=4:set expandtab=on:
