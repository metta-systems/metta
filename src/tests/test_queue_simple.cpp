//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
/**
 * \brief Test queue<T> for simple types.
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

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
