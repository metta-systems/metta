/**
 * \brief Test BitArray.
 */

/*============================================================================*/

#define BOOST_AUTO_TEST_MAIN
#include <boost/test/floating_point_comparison.hpp>
#include <boost/test/included/unit_test.hpp>
// #include <boost/test/unit_test.hpp>

#include "BitArray.h"

BOOST_AUTO_TEST_CASE(test_BitArray)
{
	BOOST_CHECK_EQUAL(BitArray::INDEX_TO_BIT(0), 0);
	BOOST_CHECK_EQUAL(BitArray::INDEX_TO_BIT(10), 320);

	BOOST_CHECK_EQUAL(BitArray::INDEX_FROM_BIT(0), 0);
	BOOST_CHECK_EQUAL(BitArray::INDEX_FROM_BIT(32), 1);
	BOOST_CHECK_EQUAL(BitArray::INDEX_FROM_BIT(35), 1);

	BOOST_CHECK_EQUAL(BitArray::OFFSET_FROM_BIT(0), 0);
	BOOST_CHECK_EQUAL(BitArray::OFFSET_FROM_BIT(32), 0);
	BOOST_CHECK_EQUAL(BitArray::OFFSET_FROM_BIT(33), 1);
	BOOST_CHECK_EQUAL(BitArray::OFFSET_FROM_BIT(35), 3);

	BitArray array(32);
	array.set(1);
	BOOST_CHECK_EQUAL(array.test(1), true);
	BOOST_CHECK_EQUAL(array.test(0), false);
	BOOST_CHECK_EQUAL(array.test(0), false);
}
