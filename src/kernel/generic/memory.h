//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2010, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "types.h"
#include "ia32.h"

template <typename T>
inline T page_align_up(T a)
{
    if (a % PAGE_SIZE)
    {
        a &= PAGE_MASK;
        a += PAGE_SIZE;
    }
    return a;
}

template <typename T>
inline T page_align_down(T a)
{
    return a - a % PAGE_SIZE;
}

template <typename T>
inline T page_align_down(void* a)
{
    const T b = reinterpret_cast<T>(a);
    return b - b % PAGE_SIZE;
}

template <typename T>
inline bool is_page_aligned(T a)
{
    return a % PAGE_SIZE == 0;
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
