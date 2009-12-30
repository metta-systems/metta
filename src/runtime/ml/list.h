//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "generic_iterator.h"

// Allocate runs of small fixed-size objects, reusing freed space as soon as possible.
// Allocates 2**factor units of sizeof(type_t) as backing storage.
// template<typename type_t, int factor>
// class fixed_small_allocator_t
// {
// };
// 
// template<typename type_t>
// class list_t
// {
//     typedef list_node_t<type_t> node_t;
//     typedef fixed_small_allocator_t<node_t, 8> allocator_t;
// };

//! @note Satisfies generic_iterator_t interface.
template<typename type_t>
class list_node_t
{
    // sequential_iterator_t interface:
    list_node_t* previous() { return previous_; }
    list_node_t* next()     { return next_; }

    list_node_t* previous_;
    list_node_t* next_;
    type_t       value;
};

template<typename T, typename Allocator, typename Behavior>
class list_t
{
    typedef list_t<T, Behavior>                                             self_type;
    typedef list_node_t<T>                                                  node_t;

public:
    typedef Behavior                                                        value_type_behavior;
    typedef Allocator                                                       allocator_t; //fixme: allocator should be allocating node_t

    typedef typename T                                                      value_type;
    typedef size_t                                                          size_type;
    typedef ptrdiff_t                                                       difference_type;
    typedef value_type*                                                     pointer;
    typedef generic_iterator<T, node_t>                                     iterator;
    typedef iterator::const_iterator                                        const_iterator;
    typedef iterator::reverse_iterator                                      reverse_iterator;
    typedef iterator::const_reverse_iterator                                const_reverse_iterator;
    typedef value_type&                                                     reference;
    typedef const value_type&                                               const_reference;

public:
    list_t(const allocator_t& _allocator = allocator_t())
        : allocator(_allocator)
        , head(0)
        , tail(0)
        , list_size(0)
    {}

    /// @returns true if list contains no items. O(1).
    bool empty() const
    {
        return !list_size;
    }

    /// @returns size of list (item count). O(1).
    const size_type& size() const
    {
        return list_size;
    }

    /// @returns item count list can store without allocating more memory. O(1)
    //! @note This implementation always allocates memory when adding list nodes.
    const size_type& capacity() const
    {
        return list_size;
    }

    /// @returns iterator to beginning of the sequence. O(1)
    iterator begin()
    {
        return head;
    }

    /// @returns const iterator to beginning of the sequence. O(1)
    const_iterator begin() const
    {
        return head;
    }

    /// @returns iterator to the end of the sequence. O(1)
    iterator end()
    {
        return iterator();
    }

    /// @returns const iterator to the end of the sequence. O(1)
    const_iterator end() const
    {
        return const_iterator();
    }

    reverse_iterator rbegin()
    {
        return tail;
    }

    const_reverse_iterator rbegin() const
    {
        return tail;
    }

    reverse_iterator rend()
    {
        return reverse_iterator();
    }

    const_reverse_iterator rend() const
    {
        return const_reverse_iterator();
    }

    /// @returns reference to the first item in the sequence. O(1)
    reference front()
    {
        return begin()->value;
    }

    /// @returns const reference to the first item in the sequence. O(1)
    const_reference front() const
    {
        return begin()->value;
    }

    /// @returns reference to the last item in the sequence. O(1)
    reference back()
    {
        return tail->value;
    }

    /// @returns const reference to the last item in the sequence. O(1)
    const_reference back() const
    {
        return tail->value;
    }

    /// @returns reference to the object stored at specified index. O(n)
    //! @todo Should throw bad_subscript exception.
    value_type& operator [](size_type index) throws()
    {
        iterator v = begin();
        while (index-- && v)
            ++v;
        return *v;
    }

    /// @returns const reference to the object stored at specified index. O(n)
    const value_type& operator [](size_type index) const throws()
    {
        const_iterator v = begin();
        while (index-- && v)
            ++v;
        return *v;
    }

    /// Appends specified value to the end of the sequence. O(1).
    void push_back(const value_type& value)
    {
        node_t* node = allocator.allocate(0, 0, 1);
        node->value = value;
        if (tail)
            tail->next = node;
        node->prev = tail;
        tail = node;
        list_size++;
    }

    /// Removes element at the back of the sequence. O(1).
    void pop_back()
    {
        if (list_size)
        {
            --list_size;
//             value_type_behavior::destructor::destruct(end(), 1);
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
    list_node_t<value_type>* head;
    list_node_t<value_type>* tail;
    size_type                list_size;
};
