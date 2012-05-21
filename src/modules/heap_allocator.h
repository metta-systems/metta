//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2012, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

inline void do_checkpoint(const char* chk) {} // for debugging the allocators.

#include <bde_allocator>
#include "heap_v1_interface.h"

namespace std {

class heap_allocator_implementation : public std::bde_allocator
{
    heap_v1::closure_t* heap;
public:
    typedef size_t size_type;

    heap_allocator_implementation(heap_v1::closure_t* h) : heap(h) 
    {
        kconsole << "** heap_allocator_impl: ctor this " << this << ", heap " << h << endl;
    }

    void* allocate(size_type __n)
    {
        kconsole << "** heap_allocator_impl: allocate " << __n << endl;
        return reinterpret_cast<void*>(heap->allocate(__n));
    }
    void deallocate(void* __p)
    { 
        kconsole << "** heap_allocator_impl: deallocate " << __p << endl;
        heap->free(reinterpret_cast<memory_v1::address>(__p));
    }
};

} // namespace std
