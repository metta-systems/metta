//
// Copyright 2007 - 2009, Renārs Lediņš <renars@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include <new> //FIXME: avoid host-includes

template <typename T>
inline void construct_inplace(T* memory, const T& value)
{
    (void)new(static_cast<void*>(memory)) T(value);
}

template <typename T>
inline void destruct_inplace(T* object)
{
    object->~T();
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
