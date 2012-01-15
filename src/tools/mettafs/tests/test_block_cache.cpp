//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2011, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
/*!
 * @brief Test block_cache_t functionality.
 */

/*============================================================================*/

#include <string.h>

#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE( mettafs )

BOOST_AUTO_TEST_CASE(block_cache_empty_on_start)
{
	block_cache_t cache(100);
	BOOST_CHECK_EQUAL(cache.allocated_size(), 0);
}

BOOST_AUTO_TEST_CASE(block_cache_readahead_stripe)
{
	block_device_t device(100);
	block_cache_t cache(device, 200);
}

BOOST_AUTO_TEST_CASE(block_cache_readahead_at_device_end)
{
}

BOOST_AUTO_TEST_CASE(block_cache_get_blocks_fails_for_too_large_request)
{
	block_cache_t cache(1);
	BOOST_CHECK_EQUAL(cache.get_blocks(2), 0);
}

BOOST_AUTO_TEST_SUITE_END()

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
