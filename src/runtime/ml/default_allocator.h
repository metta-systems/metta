//
// Copyright 2007 - 2010, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

template <typename T>
struct default_allocator
{
    typedef T value_type;

    value_type* allocate(value_type* old_mem, size_t old_size, size_t new_size)
    {
        ASSERT(old_mem == 0 && old_size == 0);
        return reinterpret_cast<value_type*>(new char [new_size]);
    }

    void deallocate(value_type* mem, size_t size)
    {
        delete reinterpret_cast<char*>(mem);
    }
};

/* Specialisation for pointers-to-objects. */
template <typename T>
struct default_allocator<T*>
{
    typedef T value_type;

    value_type* allocate(value_type* old_mem, size_t old_size, size_t new_size)
    {
        ASSERT(old_mem == 0 && old_size == 0);
        return reinterpret_cast<value_type*>(new char [new_size]);
    }
    
    void deallocate(value_type* mem, size_t size)
    {
        delete reinterpret_cast<char*>(mem);
    }
};

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
