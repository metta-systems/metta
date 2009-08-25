//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "types.h"

void* operator new(size_t size);
void* operator new(size_t size, uint32_t place);
void* operator new(size_t size, bool page_align, address_t* phys_addr = NULL);
void* operator new[](size_t size);
void* operator new[](size_t size, bool page_align, address_t* phys_addr = NULL);
void  operator delete(void* p);
void  operator delete[](void* p);

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
