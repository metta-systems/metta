//
// Copyright 2007 - 2009, Renārs Lediņš <renars@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

template <typename Iterator, typename ValueType, typename DiffType>
class reverse_iterator
{
    typedef reverse_iterator<Iterator, ValueType, DiffType>     self_type;

public:
    typedef Iterator                                            iterator;
    typedef ValueType                                           value_type;
    typedef DiffType                                            difference_type;

    typedef value_type&                                         reference;
    typedef value_type*                                         pointer;

    reverse_iterator(const iterator& _iterator)
        : m_iterator(_iterator)
    {}

    reference operator *() const
    {
        return *(m_iterator - 1);
    }

    pointer operator ->() const
    {
        return m_iterator - 1;
    }

    self_type& operator ++()
    {
        --m_iterator;
        return *this;
    }

    self_type& operator ++(int)
    {
        self_type tmp = *this; //FIXME: reference to stack object?
        --m_iterator;
        return tmp;
    }

    self_type& operator --()
    {
        ++m_iterator;
        return *this;
    }

    self_type operator --(int) // missing & ?
    {
        self_type tmp = *this; //FIXME: reference to stack object?
        ++m_iterator;
        return tmp;
    }

    self_type operator +(difference_type n) const
    {
        return self_type(m_iterator - n);
    }

    self_type& operator +=(difference_type n)
    {
        m_iterator -= n;
        return *this;
    }

    self_type operator -(difference_type n) const
    {
        return self_type(m_iterator + n);
    }

    self_type& operator -=(difference_type n)
    {
        m_iterator += n;
        return *this;
    }

    reference operator [](difference_type n) const
    { 
        return *(m_iterator + n);
    }

private:
    iterator m_iterator;
};
