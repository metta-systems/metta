//
// Copyright 2007 - 2008, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
/**
 * \brief Test bit_array.
 */

/*============================================================================*/

#define BOOST_AUTO_TEST_MAIN
#include <boost/test/floating_point_comparison.hpp>
#include <boost/test/included/unit_test.hpp>
// #include <boost/test/unit_test.hpp>

#include "BitArray.h"
using metta::common::bit_array;

BOOST_AUTO_TEST_CASE(test_bit_array)
{
	BOOST_CHECK_EQUAL(bit_array::INDEX_TO_BIT(0), 0);
	BOOST_CHECK_EQUAL(bit_array::INDEX_TO_BIT(10), 320);

	BOOST_CHECK_EQUAL(bit_array::INDEX_FROM_BIT(0), 0);
	BOOST_CHECK_EQUAL(bit_array::INDEX_FROM_BIT(32), 1);
	BOOST_CHECK_EQUAL(bit_array::INDEX_FROM_BIT(35), 1);

	BOOST_CHECK_EQUAL(bit_array::OFFSET_FROM_BIT(0), 0);
	BOOST_CHECK_EQUAL(bit_array::OFFSET_FROM_BIT(32), 0);
	BOOST_CHECK_EQUAL(bit_array::OFFSET_FROM_BIT(33), 1);
	BOOST_CHECK_EQUAL(bit_array::OFFSET_FROM_BIT(35), 3);

	bit_array array(32);
	array.set(1);
	BOOST_CHECK_EQUAL(array.test(1), true);
	BOOST_CHECK_EQUAL(array.test(0), false);
	BOOST_CHECK_EQUAL(array.test(0), false);
}

// kate: indent-width 4; replace-tabs on;
// vi:set ts=4:set expandtab=on:
