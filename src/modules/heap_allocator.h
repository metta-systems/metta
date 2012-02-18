#pragma once

#include <memory>
#include "heap_v1_interface.h"

namespace std {

class heap_allocator_implementation : public std::allocator_implementation
{
    heap_v1::closure_t* heap;
public:
    typedef size_t size_type;

    heap_allocator_implementation(heap_v1::closure_t* h) : heap(h) 
    {
        kconsole << "** heap_allocator_impl: ctor this " << this << ", heap " << h << endl;
    }

    void* allocate(size_type __n, void* = 0)
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

inline void do_checkpoint(const char* chk) {} // for debugging the allocators.

} // namespace std
