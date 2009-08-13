//
// Copyright 2007 - 2009, Renārs Lediņš <renars@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "obj_move.h"

template <typename T>
struct obj_mover
{
    static void move(T* dest, T* src, size_t count)
    {
        obj_move(dest, src, count);
    }
};
