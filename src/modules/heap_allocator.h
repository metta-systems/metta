//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2012, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include <memory>
#include "heap_v1_interface.h"

namespace std {

template <class T>
class heap_allocator : public std::allocator<T>
{
    heap_v1::closure_t* heap;
public:
    inline heap_v1::closure_t* get_state() const { return heap; } // accessor for rebind copy ctor

    typedef size_t                                size_type;
    typedef ptrdiff_t                             difference_type;
    typedef T*                                    pointer;
    typedef const T*                              const_pointer;
    // typedef typename add_lvalue_reference<T>::type       reference;
    // typedef typename add_lvalue_reference<const T>::type const_reference;
    typedef T                                     value_type;

    template <class U> struct rebind {typedef heap_allocator<U> other;};

    heap_allocator() throw()
        : heap(0)
    {
        kconsole << "default constructing heap_allocator" << endl;
    }

    heap_allocator(heap_v1::closure_t* h) throw()
        : heap(h)
    {
        kconsole << "constructing heap_allocator with heap " << heap << endl;
    }

    heap_allocator(const heap_allocator& other) throw()
        : heap(other.get_state())
    {
        kconsole << "copy constructing heap_allocator with heap " << heap << endl;
    }

    template <class U> heap_allocator(const heap_allocator<U>& other) throw()
        : heap(other.get_state())
    {        
        kconsole << "rebind copy constructing heap_allocator with heap " << heap << endl;
    }

    ~heap_allocator()
    {
        kconsole << "destructing heap_allocator" << endl;
    }

    pointer allocate(size_type s, std::allocator<void>::const_pointer hint = 0)
    {
        kconsole << "heap_allocator::allocate " << s << ": from heap " << heap << endl;
        return reinterpret_cast<pointer>(heap->allocate(s));
    }
    void deallocate(pointer p, size_type n) throw()
    {
        kconsole << "heap_allocator::deallocate " << n << " @ " << p << ": in heap " << heap << endl;
        heap->free(reinterpret_cast<memory_v1::address>(p));
    }
};

/*  
    pointer address(reference x) const noexcept;
    const_pointer address(const_reference x) const noexcept;

    size_type max_size() const noexcept;
    template<class U, class... Args>
        void construct(U* p, Args&&... args);
    template <class U>
        void destroy(U* p);
};
*/

} // namespace std
