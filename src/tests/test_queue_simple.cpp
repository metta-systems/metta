//
// Part of Metta OS. Check http://atta-metta.net for latest version.
//
// Copyright 2007 - 2012, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
/**
 * @brief Test queue<T> for simple types.
 */

/*============================================================================*/

#include <string.h>

#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include "queue.h"
using namespace metta::common;

BOOST_AUTO_TEST_SUITE( core_classes )

BOOST_AUTO_TEST_CASE(test_queue)
{
    queue<int> q;

    BOOST_CHECK_EQUAL(q.empty(), true);
}

BOOST_AUTO_TEST_SUITE_END()
