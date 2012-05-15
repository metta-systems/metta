//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2012, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Implementation of memory manipulation utilities for case when there's no standard C library.
//
#include "memutils.h"

// stdlib compat for compiler
extern "C" void* memcpy(void* dest, const void* src, size_t count) 
{ 
    return memutils::copy_memory(dest, src, count);
}

extern "C" void* memset(void *dest, int value, size_t count)
{
	return memutils::fill_memory(dest, value, count);
}
