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

//! Return bytes needed to align @c addr to next @c size boundary. Size must be power of 2.
template <typename T, typename U>
inline T align_bytes(T addr, U size)
{
    return (size - (addr & (size - 1))) & (size - 1);
}

//! Align @c addr up to next @c size boundary and return. Size must be power of 2.
template <typename T, typename U>
inline T align_up(T addr, U size)
{
    return (addr + size - 1) & ~(size - 1);
}

//! Align @c addr up to next page size boundary and return.
template <typename T>
inline T page_align_up(T addr)
{
    return align_up(addr, PAGE_SIZE);
}

//! Align @c addr down to previous page size boundary and return.
template <typename T>
inline T page_align_down(T addr)
{
    return addr - addr % PAGE_SIZE;
}

//! Align @c addr down to previous page size boundary and return.
template <typename T>
inline T page_align_down(void* addr)
{
    const T b = reinterpret_cast<T>(addr);
    return b - b % PAGE_SIZE;
}

//! Check that @c addr is aligned to page boundary.
template <typename T>
inline bool is_page_aligned(T addr)
{
    return (addr & (PAGE_SIZE - 1)) == 0;
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
