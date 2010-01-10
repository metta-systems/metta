//
// Copyright 2007 - 2010, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "rbtree.h"

/*!
 * Map is associative structure mapping keys of type K to values of type V.
 * Dictionary is specialization of map mapping string keys to values of arbitrary type V.
 */
template <typename K, typename V>
class map_t
{
//     template <typename K, typename V>
    struct pair_t
    {
        typedef pair_t/*<K,V>*/ self_type;

        K key;
        V value;

        pair_t(K k, V v) : key(k), value(v) {}
        bool operator <(self_type other) const { return key < other.key; }
        bool operator <=(self_type other) const { return key <= other.key; }
        bool operator >(self_type other) const { return key > other.key; }
        bool operator >=(self_type other) const { return key >= other.key; }
        bool operator ==(self_type other) const { return key == other.key; }
    };

    rbtree_t<pair_t> data;

public:
    map_t() : data() {}

    inline void insert(K k, V v) { data.insert(pair_t(k,v)); }
//     V search(K k) { V ret; if(data.search(pair_t(k,ret))) return ret; else return V::null; }
};

// template <typename V>
// class dict_t : public map_t<string, V>
// {
// };
