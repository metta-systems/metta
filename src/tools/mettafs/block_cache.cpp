#include "block_cache.h"
#include "panic.h"
#include <string.h> // memcpy
#include <cstdio>
#include <cassert>

//=====================================================================================================================
// System-dependent functions to read and write blocks for now...
//=====================================================================================================================

size_t block_cache_t::read_blocks(deviceno_t device, block_device_t::blockno_t block_n, void* data, size_t nblocks, size_t block_size)
{
	// FIXME: replace with accesses to block_device_t
	lseek(device, block_n * block_size, SEEK_SET);
	ssize_t ret = read(device, data, nblocks * block_size);
	return (ret == -1) ? 0 : (ret / block_size);
}

size_t block_cache_t::write_blocks(deviceno_t device, block_device_t::blockno_t block_n, const void* data, size_t nblocks, size_t block_size)
{
	// FIXME: replace with accesses to block_device_t
	lseek(device, block_n * block_size, SEEK_SET);
	ssize_t ret = write(device, data, nblocks * block_size);
	return (ret == -1) ? 0 : (ret / block_size);
}

//=====================================================================================================================
// cache_block_t
//=====================================================================================================================

cache_block_t::cache_block_t(deviceno_t dev, block_device_t::blockno_t block_n, size_t size)
	: device(dev)
	, block_num(block_n)
	, block_size(size)
	, dirty(false)
	, busy(false)
	, next_mru(0)
	, prev_lru(0)
{
	data = new char [size];
}

cache_block_t::~cache_block_t()
{
	delete data;
}

// Double-linked list helper functions.

// Link at MRU side.
//  +--------------+         +--------------+   +------------+
//  + prev_lru     +-\       + prev_lru     +-->+ prev_lru=0 |
//  +--------------+  >MRU-> +--------------+   +------------+ <--LRU
//  + next_mru     +<--------+ next_mru = 0 +<--+ next_mru   |
//  +--------------+         +--------------+   +------------+
//
void cache_block_t::link_at_mru(cache_block_list_t* parent)
{
	assert(prev_lru == 0);
	assert(next_mru == 0);

	if (parent->mru)
	{
		prev_lru = parent->mru;
		parent->mru->next_mru = this;
	}
	parent->mru = this;
}

// Link at LRU side.
//        +--------------+   +------------+          +--------------+
//        + prev_lru     +-->+ prev_lru=0 |--------->+ prev_lru     +
//  MRU-> +--------------+   +------------+ <--LRU<  +--------------+
//        + next_mru = 0 +<--+ next_mru   |        \-+ next_mru = 0 +
//        +--------------+   +------------+          +--------------+
//
void cache_block_t::link_at_lru(cache_block_list_t* parent)
{
	assert(prev_lru == 0);
	assert(next_mru == 0);

	if (parent->lru)
	{
		next_mru = parent->lru;
		parent->lru->prev_lru = this;
	}
	parent->lru = this;
}

void cache_block_t::unlink(cache_block_list_t* parent)
{
	if (prev_lru)
		prev_lru->next_mru = next_mru;
	if (next_mru)
		next_mru->prev_lru = prev_lru;
	
	if (parent->lru == this)
		parent->lru = parent->lru->next_mru;
	if (parent->mru == this)
		parent->mru = parent->mru->prev_lru;

	prev_lru = next_mru = 0;
}

//=====================================================================================================================
// Portable block cache implementation.
//=====================================================================================================================

block_cache_t::~block_cache_t()
{
}

/*!
 * Find the block in the cache.
 * Assumes all necessary locks are held (acts as internal worker function).
 * If block is busy doing I/O will wait a certain amount of time (not implemented for single-threaded test).
 */
cache_block_t* block_cache_t::block_lookup(deviceno_t device, block_device_t::blockno_t block_n)
{
	block_id_t id = std::make_pair(device, block_n);
	cache_map_t::iterator it;

	if ((it = cache.find(id)) != cache.end())
	{
		//TODO: check for busy block, and wait if it is busy (shouldn't happen in single threaded test).
		return (*it).second;
	}
	return NULL;
}

/*!
 * Blocks we are trying to get may be either busy, locked or dirty. In either case, we cannot discard them and reuse for anything.
 * We also need to keep track that the blocks we've taken are of correct size, and if not - reallocate them.
 */
std::vector<cache_block_t*> block_cache_t::get_blocks(size_t nblocks, size_t block_size)
{
	if (nblocks > max_blocks)
		PANIC("Cannot allocate more blocks than allowed in the cache in total!");

	std::vector<cache_block_t*> ret;

	// If cache is not filled, just allocate new blocks.
	size_t nfree = max_blocks - cache.size();
	while (nfree && nblocks)
	{
		cache_block_t* blk = new cache_block_t(-1, -1, block_size);
		assert(blk); // FIXME: do a real check.
		ret.push_back(blk);
		--nblocks;
		--nfree;
	}

	// We've exhausted the cache free space, now take old entries off the cache and reuse.
	while (nblocks)
	{
		cache_block_t* blk = blocks.lru;
		while (blk)
		{
			if (!blk->is_usable() || (blk->size() != block_size))
			{
				blk = blk->next_mru;
				continue;
			}
			blk->unlink(&blocks);
			ret.push_back(blk);
			--nblocks;
			break;
		}
	}

	return ret;
}

size_t block_cache_t::byte_read(deviceno_t device, off_t byte_offset, void* data, size_t nbytes)
{
	return -1;
}

size_t block_cache_t::byte_write(deviceno_t device, off_t byte_offset, const void* data, size_t nbytes)
{
	return -1;
}

/*!
 * @returns number of blocks successfully read.
 */
size_t block_cache_t::cached_read(deviceno_t device, block_device_t::blockno_t block_n, void* data, size_t nblocks, size_t block_size)
{
	cache_block_t* entry(0);
	char* buffer = static_cast<char*>(data);
	size_t actually_read;

	if (nblocks * block_size > 64*1024)
	{
		// Large read: do directly!
		actually_read = read_blocks(device, block_n, buffer, nblocks, block_size);
		// Update read data with contents of dirty blocks in cache if necessary.
		while (nblocks)
		{
			entry = block_lookup(device, block_n);
			if (entry)
			{
				assert(entry->block_size == block_size);
				if (entry->dirty)
					memcpy(entry->data, buffer, block_size); // Update read data with cache data (e.g. dirty blocks).
			}
			block_n++;
			nblocks--;
			buffer += block_size;	
		}
		return actually_read;
	}

	// Small reads, do slower block-by-block for now.
	while (nblocks)
	{
		entry = block_lookup(device, block_n);
		if (entry)
		{
			assert(entry->block_size == block_size);
			// Block is found in cache.
			entry->unlink(&blocks); // Remove it from the list it is in, because it's going to be modified.
			memcpy(buffer, entry->data, block_size); // FIXME: replace this with a visitor pattern?
			// Add block back at the start of the MRU list.
			entry->link_at_mru(&blocks);

			block_n++;
			nblocks--;
			buffer += block_size;
		}
		else
		{
			// Block is not found in the cache, need to perform a read (preferably with a readahead, for at least number of blocks requested)
			// Check if some blocks in that range are already in the cache anyway?
			// FIXME: Would it be simpler to read entire stripes regardless and then just discard blocks already in the cache?

			// find how many adjacent blocks from the request are not in the cache, to read them all at once
			block_device_t::blockno_t block_stripe;
			for (block_stripe = 1; block_stripe < nblocks; ++block_stripe) // start from 1, since block 0 definitely not found (above).
			{
				if (block_lookup(device, block_n + block_stripe))
					break;
			}

			actually_read = read_blocks(device, block_n, buffer, block_stripe, block_size);

			if (actually_read < block_stripe)
				PANIC("Read blocks from physical media failed! [make it nonfatal]");

			// create new blocks for just read data
			// add new block to the cache, evicting LRU entries as needed
			auto ents = get_blocks(block_stripe, block_size);

			if (ents.size() < block_stripe)
				PANIC("Couldn't get enough block cache entries.");

			// cache the blocks
			// for (size_t b = 0; b < block_stripe; ++b)

			// keep looping
		}
	}
	return 0;
}

size_t block_cache_t::cached_write(deviceno_t device, block_device_t::blockno_t block_n, const void* data, size_t nblocks, size_t block_size)
{
	cache_block_t* entry(0);
	char* buffer = static_cast<char*>(const_cast<void*>(data));

	while (nblocks)
	{
		entry = block_lookup(device, block_n);
		if (entry)
		{
			assert(entry->block_size == block_size);
			// Block is found in cache.
			entry->unlink(&blocks); // Remove it from the list it is in, because it's going to be modified.
			memcpy(entry->data, buffer, block_size); // FIXME: replace this with a visitor pattern?

			entry->set_dirty();

			// Add block back at the start of the MRU list.
			entry->link_at_mru(&blocks);
		}
		else
		{
			// Block is not found in the cache, create a new one (potentially pushing older blocks out of cache).
			auto ents = get_blocks(1, block_size);
			// entry = new cache_block_t(device, block_n, block_size);
			// memcpy(entry->data, buffer, block_size); // FIXME: replace this with a visitor pattern?
			// entry->set_dirty();
			// add_new_block(entry);
		}

		block_n++;
		nblocks--;
		buffer += block_size;
	}

	return 0;
}
