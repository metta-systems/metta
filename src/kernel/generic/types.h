//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2010, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include <stdint.h> // Use C99-mandated types.

#ifdef NULL
#undef NULL
#endif

#define NULL 0

typedef uint32_t size_t;
typedef uint32_t addr_t;
typedef int32_t  ptrdiff_t;
typedef uintptr_t address_t;          //!< Virtual address type.
typedef uintptr_t physical_address_t; //!< Physical address type. Used in physical memory allocation.
//typedef uint32_t hash_t;
typedef uint32_t flags_t;

// Endian-aware types - FIXME: need wrapper classes that can do endianness-conversions.
//typedef uint32_t uint32_le_t;
//typedef uint32_t uint32_be_t;
// etc

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
