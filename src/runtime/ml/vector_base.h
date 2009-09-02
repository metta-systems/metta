//
// Copyright 2007 - 2009, Renārs Lediņš <renars@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "types.h"
#include "reverse_iterator.h"

struct tight_allocation_policy
{
    static size_t get_size(size_t size, size_t)
    {
        return size;
    }
};

struct stl_allocation_policy
{
    static size_t get_size(const size_t& size, const size_t& capacity)
    {
        return size < capacity ? size : ((capacity + 1) * 2);
    }
};

template <typename Memblock, typename Behavior, typename AllocationPolicy>
class vector_base
{
    typedef vector_base<Memblock, Behavior, AllocationPolicy>               self_type;

public:
    typedef Memblock                                                        memblock;
    typedef AllocationPolicy                                                allocation_policy;
    typedef Behavior                                                        value_type_behavior;

    typedef typename memblock::allocator                                    allocator;

    typedef typename memblock::value_type                                   value_type;
    typedef size_t                                                          size_type;
    typedef ptrdiff_t                                                       difference_type;
    typedef value_type*                                                     pointer;
    typedef value_type*                                                     iterator;
    typedef const value_type*                                               const_iterator;
    typedef reverse_iterator<const_iterator, value_type, difference_type>   const_reverse_iterator;
    typedef reverse_iterator<iterator, value_type, difference_type>         reverse_iterator;
    typedef value_type&                                                     reference;
    typedef const value_type&                                               const_reference;

public:
    vector_base(const allocator& _allocator = allocator())
        : m_memblock(_allocator)
        , m_size(0)
    {}

    /// @returns true if vector container contains no items. O(1).
    bool empty() const
    {
        return !m_size;
    }

    /// @returns size of vector (item count). O(1).
    const size_type& size() const
    {
        return m_size;
    }

    /// @returns item count vector can store without allocating more memory.
    const size_type& capacity() const
    {
        return m_memblock.capacity();
    }

    /// @returns iterator to beginning of the sequence.
    iterator begin()
    {
        return m_memblock.ptr();
    }

    /// @returns const iterator to beginning of the sequence.
    const_iterator begin() const
    {
        return m_memblock.ptr();
    }

    /// @returns iterator to the end of the sequence.
    iterator end()
    {
        return m_memblock.ptr() + m_size;
    }

    /// @returns const iterator to the end of the sequence.
    const_iterator end() const
    {
        return m_memblock.ptr() + m_size;
    }

    reverse_iterator rbegin()
    {
        return end();
    }

    const_reverse_iterator rbegin() const
    {
        return end();
    }

    reverse_iterator rend()
    {
        return begin();
    }

    const_reverse_iterator rend() const
    {
        return begin();
    }

    /// @returns reference to the first item in the sequence.
    reference front()
    {
        return *begin();
    }

    /// @returns const reference to the first item in the sequence.
    const_reference front() const
    {
        return *begin();
    }

    /// @returns reference to the last item in the sequence.
    reference back()
    {
        return *(end() - 1);
    }

    /// @returns const reference to the last item in the sequence.
    const_reference back() const
    {
        return *(end() - 1);
    }

    /// @returns reference to the object stored at specified index.
    value_type& operator [](size_type index)
    {
        return m_memblock.ptr()[index];
    }

    /// @returns const reference to the object stored at specified index.
    const value_type& operator [](size_type index) const
    {
        return m_memblock.ptr()[index];
    }

    /// Appends specified value to the end of the sequence. O(1).
    /**
     Reallocates memory if necessary. O(n).
    */
    void push_back(const value_type& value)
    {
        reserve(allocation_policy::get_size(m_size + 1, capacity()));
        construct_inplace(begin() + m_size++, value);
    }

    /// Removes element at the back of the sequence. O(1).
    void pop_back()
    {
        if (m_size)
        {
            --m_size;
            value_type_behavior::destructor::destruct(end(), 1);
        }
    }

    /// Inserts specified value at specified position in sequence. O(n).
    void insert(size_type pos, const value_type& value);

    void insert(iterator pos, const value_type& value)
    {
        insert(pos - begin(), value);
    }

    /// Removes value at specified position in sequence. O(n).
    void erase(size_type pos);

    /// Preallocates memory so that vector can store at least specified amount of items.
    void reserve(size_type reserve_size)
    {
        m_memblock.allocate(reserve_size);
    }

    /// Releases memory allocated for data storage and clears vector.
    void destroy()
    {
        m_memblock.deallocate();
        m_size = 0;
    }

    /// Clears vector object.
    void clear()
    {
        value_type_behavior::destructor::destruct(begin(), m_size);
        m_size = 0;
    }

    self_type& operator =(const self_type& other)
    {
        if (this != &other)
            copy_from(other);
        return *this;
    }

private:
    /// Copies data from other vector object.
    void copy_from(const self_type& other);

private:
    memblock        m_memblock;
    size_type       m_size;
};

template <typename Memblock, typename Behavior, typename AllocationPolicy>
void vector_base<Memblock, Behavior, AllocationPolicy>::copy_from(const self_type& other)
{
    m_memblock  = other.m_memblock;
    m_size      = other.m_size;
}

template <typename Memblock, typename Behavior, typename AllocationPolicy>
void vector_base<Memblock, Behavior, AllocationPolicy>::insert(size_type pos, const value_type& value)
{
    if (pos < m_size)
    {
        reserve(allocation_policy::get_size(m_size + 1, capacity()));
        iterator offset = begin() + pos;
        value_type_behavior::mover::move(offset + 1, offset, m_size - pos);
        construct_inplace(offset, value);
        m_size++;
    }
    else
        push_back(value);
}

template <typename Memblock, typename Behavior, typename AllocationPolicy>
void vector_base<Memblock, Behavior, AllocationPolicy>::erase(size_type pos)
{
    if (m_size == 1)
        clear();
    else if (pos < m_size)
    {
        iterator offset = begin() + pos;
        value_type_behavior::destructor::destruct(offset);
        value_type_behavior::mover::move(offset, offset + 1, m_size - pos);
        --m_size;
    }
    else
        pop_back();
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
