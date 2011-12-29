/*!
 * Filesystem block cache.
 *
 * Block cache handles device reads and writes and adds faster access for reading and delayed buffer management for writing.
 * It is very simple - blocks are entered into a map based on block number and origin device, blocks are also entered into a LRU/MRU list,
 * for easy eviction if memory pressure increases.
 */
#pragma once

#include "block_device.h"
#include <map>
#include <vector>

class cache_block_t;

typedef uint32_t deviceno_t;

struct cache_block_list_t
{
	cache_block_t* mru;
	cache_block_t* lru;	
};

class cache_block_t
{
	deviceno_t device;
	block_device_t::blockno_t block_num;

	size_t block_size;
	char* data;

	bool dirty; //!< Block is dirty (written to but not flushed yet).
	bool busy; //!< Block is busy (I/O operation in progress).

	cache_block_t* next_mru; //!< Points towards MRU end of the list.
	cache_block_t* prev_lru; //!< Points towards LRU end of the list.

	friend class block_cache_t;

public:
	cache_block_t(deviceno_t device, block_device_t::blockno_t block_n, size_t size);
	~cache_block_t();

	void set_dirty() { dirty = true; }
	bool is_usable() { return !dirty && !busy/* && !locked*/; }
	size_t size() { return block_size; }

	void link_at_mru(cache_block_list_t* parent);
	void link_at_lru(cache_block_list_t* parent);
	void unlink(cache_block_list_t* parent);
};

class block_cache_t
{
	typedef std::pair<deviceno_t, block_device_t::blockno_t> block_id_t; //!< Pairs device number with block number.
	typedef std::map<block_id_t, cache_block_t*> cache_map_t;
	cache_map_t cache; //!< Map for quickly finding blocks given device and block number pair.
	std::map<deviceno_t, size_t> max_device_blocks; //!< Maximum number of blocks in each opened device (for error checking).
	size_t max_blocks; //!< Maximum number of blocks stored in this cache.
	cache_block_list_t blocks; //!< LRU list of blocks.

	/*!
	 * Perform actual read on physical blocks.
	 * @return number of blocks successfully read or 0 on failure.
	 */
	size_t read_blocks(deviceno_t device, block_device_t::blockno_t block_n, void* data, size_t nblocks, size_t block_size);
	/*!
	 * Perform actual write on physical blocks.
	 * @return number of blocks successfully written or 0 on failure.
	 */
	size_t write_blocks(deviceno_t device, block_device_t::blockno_t block_n, const void* data, size_t nblocks, size_t block_size);

	cache_block_t* block_lookup(deviceno_t device, block_device_t::blockno_t block_n);

	/*!
	 * Obtain a number of blocks by evicting oldest blocks from the cache.
	 */
	std::vector<cache_block_t*> get_blocks(size_t nblocks, size_t block_size);

public:
	/*!
	 * Create a cache that can store maximum of n_blocks data blocks.
	 */
	block_cache_t(size_t n_blocks) : max_blocks(n_blocks) { blocks.lru = blocks.mru = NULL; }
	~block_cache_t();

	size_t cached_read(deviceno_t device, block_device_t::blockno_t block_n, void* data, size_t nblocks, size_t block_size);
	size_t cached_write(deviceno_t device, block_device_t::blockno_t block_n, const void* data, size_t nblocks, size_t block_size);

	// helper functions for the vfs layer, they will figure out the block size themselves
	size_t byte_read(deviceno_t device, off_t byte_offset, void* data, size_t nbytes);
	size_t byte_write(deviceno_t device, off_t byte_offset, const void* data, size_t nbytes);
};
