//
// Part of Metta OS. Check http://atta-metta.net for latest version.
//
// Copyright 2007 - 2012, Stanislav Karchebnyy <berkus@exquance.com>
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
