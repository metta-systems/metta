//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2012, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "types.h"

void* operator new(size_t size) throw();
//void* operator new(size_t size, bool page_align, address_t* phys_addr = NULL);
void* operator new[](size_t size) throw();
//void* operator new[](size_t size, bool page_align, address_t* phys_addr = NULL);
void  operator delete(void* p) throw();
void  operator delete[](void* p);

// placement new and delete
inline void* operator new(size_t, void* place) throw() { return place; }
inline void* operator new[](size_t, void* place) throw() { return place; }

inline void operator delete(void*, void*) throw() {}
inline void operator delete[](void*, void*) throw() {}

// stdlib functions to get rid of (used by silly stl!)
extern "C" void* malloc(size_t size);
extern "C" void free(void*);
extern "C" void *realloc(void *ptr, size_t size);
