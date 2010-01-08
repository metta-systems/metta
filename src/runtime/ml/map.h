#pragma once

#include "rbtree.h"

/*!
 * Map is associative structure mapping keys of type K to values of type V.
 * Dictionary is specialization of map mapping string keys to values of arbitrary type V.
 */
template <typename K, typename V>
class map_t
{
    template <typename K, typename V>
    struct pair_t
    {
        typedef pair_t<K,V> self_type;

        K key;
        V value;

        pair_t(K k, V v) : key(k), value(v) {}
        bool operator <(self_type other) { return key < other.key; }
        bool operator <=(self_type other) { return key <= other.key; }
        bool operator >(self_type other) { return key > other.key; }
        bool operator >=(self_type other) { return key >= other.key; }
        bool operator ==(self_type other) { return key == other.key; }
    };

    rbtree_t<pair_t<K,V>> data;
};

// template <typename V>
// class dict_t : public map_t<string, V>
// {
// };
