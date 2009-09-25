//
// Copyright 2007 - 2009, Renārs Lediņš <renars@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include <stdlib.h> // FIXME: avoid host includes

template <typename T>
struct pod_allocator
{
    typedef T   value_type;

    value_type* allocate(value_type* mem, size_t, size_t size)
    {
        return static_cast<value_type*>(realloc(mem, size * sizeof(value_type)));
    }

    void deallocate(value_type* mem, size_t)
    {
        free(mem);
    }
};

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
