//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2011, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "heap_v1_interface.h"

void* operator new(size_t size, heap_v1::closure_t* heap) throw();
void* operator new[](size_t size, heap_v1::closure_t* heap) throw();
void operator delete(void* p, heap_v1::closure_t* heap) throw();
void operator delete[](void* p, heap_v1::closure_t* heap) throw();

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
