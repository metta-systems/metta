//
// Copyright 2007 - 2009, Renārs Lediņš <renars@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "types.h"

template <typename T, typename Allocator, typename Copier>
class memblock_base
{
    typedef memblock_base<T, Allocator, Copier> self_type;

public:
    typedef T                                   value_type;
    typedef Allocator                           allocator;
    typedef Copier                              copier;

public:
    memblock_base(const allocator& _allocator = allocator())
        : m_allocator(_allocator)
        , m_data(0)
        , m_capacity(0)
    {}

    memblock_base(const self_type& other)
        : m_allocator(other.m_allocator)
        , m_data(0)
        , m_capacity(0)
    {
        copy_from(other);
    }

    ~memblock_base()
    {
        deallocate();
    }

    const size_t& capacity() const
    {
        return m_capacity;
    }

    bool allocate(size_t size)
    {
        if (size > m_capacity)
        {
            value_type* data = m_allocator.allocate(m_data, m_capacity, size);
            if (!data)
                return false;
            m_data      = data;
            m_capacity  = size;
            return true;
        }
        return true;
    }

    void deallocate()
    {
        if (m_capacity) 
        {
            m_allocator.deallocate(m_data, m_capacity);
            m_data      = 0;
            m_capacity  = 0;
        }
    }

    self_type& operator =(const self_type& other)
    {
        if (this != &other)
        {
            copy_from(other);
        }
        return *this;
    }

    value_type* ptr()
    {
        return m_data;
    }

    const value_type* ptr() const
    {
        return m_data;
    }

private:
    bool copy_from(const self_type& other)
    {
        if (allocate(other.m_capacity))
        {
            copier::copy(m_data, other.m_data, other.m_capacity);
            return true;
        }
        return false;
    }

private:
    allocator   m_allocator;
    value_type* m_data;
    size_t      m_capacity;
};
