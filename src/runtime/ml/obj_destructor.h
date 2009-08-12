//
// Copyright 2007 - 2009, Renārs Lediņš <renars@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "types.h"
#include "memory.h"

template <typename T>
struct obj_destructor
{
    static void destruct(T* ptr, size_t count)
    {
        while (count--)
            destruct_inplace(ptr++);
    }
};
