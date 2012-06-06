//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2012, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include <stdint.h> // Use C99-mandated types.
#include "required_features.h"

#ifdef NULL
#undef NULL
#endif

#define NULL 0L

#if __Metta__
typedef __typeof__(sizeof(int)) size_t;
// typedef uint32_t size_t;  // 32 bits build...
typedef int32_t  ptrdiff_t;
typedef ptrdiff_t offset_t;
#else
#include <cstddef>
#include <unistd.h>
#endif

typedef uint32_t addr32_t; // This is for tool support on 64 bits hosts.

typedef uintptr_t address_t;          //!< Virtual address type.
typedef uintptr_t physical_address_t; //!< Physical address type. Used in physical memory allocation.
//typedef uint32_t hash_t;
typedef uint32_t flags_t;

// Endian-aware types - FIXME: need wrapper classes that can do endianness-conversions.
//typedef uint32_t uint32_le_t;
//typedef uint32_t uint32_be_t;
// etc
