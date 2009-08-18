//
// Copyright 2007 - 2009, Renārs Lediņš <renars@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "memutils.h"

template <typename T>
void pod_copy(T* dest, T* src, size_t count)
{
    memutils::copy_memory(dest, src, count * sizeof(T));
}
