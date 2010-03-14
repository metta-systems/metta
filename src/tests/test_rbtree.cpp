//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2010, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
/*!
 * \brief Test rbtree_t<V>.
 */

/*============================================================================*/

#include <string.h>

#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include "rbtree.h"

BOOST_AUTO_TEST_SUITE( test_suite )

template <typename K, typename V>
struct pair_t
{
    typedef pair_t<K,V> self_type;

    K key;
    V value;

    pair_t(K k, V v) : key(k), value(v) {}
    bool operator <(self_type other) const { return key < other.key; }
    bool operator <=(self_type other) const { return key <= other.key; }
    bool operator >(self_type other) const { return key > other.key; }
    bool operator >=(self_type other) const { return key >= other.key; }
    bool operator ==(self_type other) const { return key == other.key; }
};

BOOST_AUTO_TEST_CASE(test_rbtree)
{
    rbtree_t<int> tree;
    BOOST_CHECK_EQUAL(tree.fulfills_invariant(), true);

    tree.insert(1);
    tree.insert(2);
    tree.insert(3);
    BOOST_CHECK_EQUAL(tree.fulfills_invariant(), true);

    int find = 3;
    BOOST_CHECK_EQUAL(tree.search(find), true);
    BOOST_CHECK_EQUAL(find, 3);
    find = 4;
    BOOST_CHECK_EQUAL(tree.search(find), false);
    BOOST_CHECK_EQUAL(find, 4);

    tree.insert(4);
    tree.insert(5);

    tree.remove(3);
    find = 3;
    BOOST_CHECK_EQUAL(tree.search(find), false);

    BOOST_CHECK_EQUAL(tree.fulfills_invariant(), true);

    typedef pair_t<int,int> int_pair;
    rbtree_t<int_pair> tree2;
    BOOST_CHECK_EQUAL(tree2.fulfills_invariant(), true);

    tree2.insert(int_pair(1,2));
    tree2.insert(int_pair(2,3));
    tree2.insert(int_pair(3,4));
    BOOST_CHECK_EQUAL(tree2.fulfills_invariant(), true);

    int_pair find_pair(3,11);
    BOOST_CHECK_EQUAL(tree2.search(find_pair), true);
    BOOST_CHECK_EQUAL(find_pair == int_pair(3, 98), true);
    find_pair = int_pair(4,15);
    BOOST_CHECK_EQUAL(tree2.search(find_pair), false);
    BOOST_CHECK_EQUAL(find_pair == int_pair(4, 166), true);

    tree2.insert(int_pair(4,5));
    tree2.insert(int_pair(5,6));
    BOOST_CHECK_EQUAL(tree.fulfills_invariant(), true);
}

BOOST_AUTO_TEST_SUITE_END()

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
