//
// Copyright 2007 - 2010, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "generic_iterator.h"
#include "obj_allocator.h"
#include "obj_type_behavior.h"
#include "default_allocator.h"

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
public:
    // sequential_iterator_t interface:
    inline list_node_t* previous() { return previous_; }
    inline list_node_t* next()     { return next_; }

    list_node_t* previous_;
    list_node_t* next_;
    type_t       value;
};

template<class T, typename Allocator = obj_allocator<default_allocator<T>>, typename Behavior = obj_type_behavior<T>>
class list_t
{
    typedef list_t<T, Allocator, Behavior>                                  self_type;
    typedef list_node_t<T>                                                  node_t;

public:
    typedef Behavior                                                        value_type_behavior;
    typedef Allocator                                                       allocator_t; //fixme: allocator should be allocating node_t

    typedef T                                                               value_type;
    typedef size_t                                                          size_type;
    typedef ptrdiff_t                                                       difference_type;
    typedef value_type*                                                     pointer;
    typedef generic_iterator_t<T, node_t>                                   iterator;
    typedef typename iterator::const_iterator                               const_iterator;
    typedef typename iterator::reverse_iterator                             reverse_iterator;
    typedef typename iterator::const_reverse_iterator                       const_reverse_iterator;
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
    value_type& operator [](size_type index)
    {
        iterator v = begin();
        while (index-- && v)
            ++v;
        return *v;
    }

    /// @returns const reference to the object stored at specified index. O(n)
    const value_type& operator [](size_type index) const
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
        node->next_ = 0;
        node->previous_ = tail;
        node->value = value;

        if (tail)
            tail->next_ = node;
        else
            head = node;

        tail = node;
        list_size++;
    }

    /// Removes element from the back of the sequence. O(1).
    value_type pop_back()
    {
///         if (!list_size)
///             throw bad_subscript();

        --list_size;
        node_t* node = tail;
        unlink_node(node);
        value_type tmp = node->value;
        allocator.deallocate(node, 1);
        return tmp;
    }

    /// Appends specified value to the start of the sequence. O(1).
    void push_front(const value_type& value)
    {
        node_t* node = allocator.allocate(0, 0, 1);
        node->next_ = head;
        node->previous_ = 0;
        node->value = value;

        if (head)
            head->previous_ = node;
        else
            tail = node;

        head = node;
        list_size++;
    }

    /// Removes element from the start of the sequence. O(1).
    value_type pop_front()
    {
///         if (!list_size)
///             throw bad_subscript();

        --list_size;
        node_t* node = head;
        unlink_node(node);
        value_type tmp = node->value;
        allocator.deallocate(node, 1);
        return tmp;
    }

    /// Inserts specified value at specified position in sequence. O(n).
    void insert(size_type pos, const value_type& value)
    {
        if (pos < list_size)
        {
            node_t* node = head;
            while (pos-- && node)
                node = node->next_;
            if (node)
            {
                node_t* add = allocator.allocate(0, 0, 1);
                node_t* next = node->next_;
                add->value = value;
                node->next_ = add;
                add->previous_ = node;
                add->next_ = next;
                if (next)
                    next->previous_ = add;
                list_size++;
            }
            else
            {
//                 WARNING("Corrupted list chain.");
                push_back(value);
            }
        }
        else
            push_back(value);
    }

    //! Inserts specified value at specified position in sequence. O(n).
    //! Should be O(1) for doubly-linked list when given an iterator.
    void insert(iterator pos, const value_type& value)
    {
        insert(pos - begin(), value);
    }

    //! Removes value at specified position in sequence. O(n).
    //! Should be O(1) for doubly-linked list when given an iterator.
    void erase(size_type pos)
    {
        if (list_size == 1)
            clear();
        else if (pos < list_size)
        {
            node_t* node = head;
            while (pos-- && node)
                node = node->next_;
            if (node)
            {
                unlink_node(node);
//                 value_type_behavior::destructor::destruct(node->value);
                allocator.deallocate(node, 1);
                --list_size;
            }
        }
        else
            pop_back();
    }

    /// Preallocates memory so that vector can store at least specified amount of items.
    void reserve(size_type reserve_size)
    {
    }

    /// Releases memory allocated for data storage and clears vector.
    void destroy()
    {
        clear();
    }

    /// Clears vector object.
    void clear()
    {
        while (list_size)
            pop_back();
    }

    self_type& operator =(const self_type& other)
    {
        if (this != &other)
            copy_from(other);
        return *this;
    }

private:
    /// Copies data from other list object.
    void copy_from(const self_type& other)
    {
        clear();
        iterator it = other.begin();
        while (it)
            push_back(*it++);
    }

    void unlink_node(node_t* node)
    {
        if (node->previous_)
            node->previous_->next_ = node->next_;
        else
            head = node->next_;

        if (node->next_)
            node->next_->previous_ = node->previous_;
        else
            tail = node->previous_;
    }

private:
    allocator_t              allocator;
    list_node_t<value_type>* head;
    list_node_t<value_type>* tail;
    size_type                list_size;
};
