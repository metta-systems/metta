//
// Copyright 2007 - 2009, Renārs Lediņš <renars@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "types.h"
#include "alloc.h"

template <typename T>
void obj_copy(T* dest, T* src, size_t count)
{
    while (count--)
    {
        destruct_inplace(dest);
        construct_inplace(dest++, *src++);
    }
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
