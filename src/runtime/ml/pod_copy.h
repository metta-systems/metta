//
// Copyright 2007 - 2009, Renars Ledins <renarspub@gmail.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include <memory.h>

template <typename T>
void pod_copy(T* dest,T* src,size_t count)
{
    memcpy(dest,src,count * sizeof(T));
}
