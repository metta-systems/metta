


void test_block_cache_empty_on_start()
{
	block_cache_t cache(100);
	EXPECT_EQ(cache.allocated_size() == 0);
}

void test_block_cache_readahead_stripe()
{
	block_device_t device(100);
	block_cache_t cache(device, 200);
		
}

void test_block_cache_readahead_at_device_end()
{
	
}

void test_block_cache_get_blocks_fails_for_too_large_request()
{
	block_cache_t cache(1);
	EXPECT_FAIL(cache.get_blocks(2));
}
