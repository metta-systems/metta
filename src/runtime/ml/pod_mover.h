//
// Copyright 2007 - 2009, Renārs Lediņš <renars@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "memutils.h"

template <typename T>
struct pod_mover
{
    static void move(T* dest, T* src, size_t count)
    {
        memutils::move_memory(dest, src, count);
    }
};

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
