//
// Part of Metta OS. Check http://atta-metta.net for latest version.
//
// Copyright 2007 - 2012, Stanislav Karchebnyy <berkus@atta-metta.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "block_cache.h"
#include "block_device_mapper.h"
#include "memutils.h"
#include <cstdio>
#include <cassert>
#include <iostream> // debug

//=====================================================================================================================
// System-dependent functions to actually read and write blocks.
//=====================================================================================================================

void block_cache_t::set_device_mapper(block_device_mapper_t& mapper)
{
    device_mapper = &mapper;
}

size_t block_cache_t::read_blocks(deviceno_t device, block_device_t::blockno_t block_n, char* data, size_t nblocks, size_t block_size)
{
    return device_mapper->read(device, block_n, data, nblocks * block_size);
}

size_t block_cache_t::write_blocks(deviceno_t device, block_device_t::blockno_t block_n, const char* data, size_t nblocks, size_t block_size)
{
    return device_mapper->write(device, block_n, data, nblocks * block_size);
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
    assert(data);
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
        if (!parent->lru)
        {
            parent->lru = this;
        }
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
        if (!parent->mru)
        {
            parent->mru = this;
        }
}

void cache_block_t::unlink_from(cache_block_list_t* parent)
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

/**
 * Find the block in the cache.
 * Assumes all necessary locks are held (acts as internal worker function).
 * If block is busy doing I/O will wait a certain amount of time (not implemented for single-threaded test).
 */
cache_block_t*
block_cache_t::block_lookup(deviceno_t device, block_device_t::blockno_t block_n)
{
    block_id_t id = std::make_pair(device, block_n);
    cache_map_t::iterator it;

    if ((it = cache.find(id)) != cache.end())
    {
        // @todo Check for busy block, and wait if it is busy
        // (shouldn't happen in single threaded test).
        return (*it).second;
    }
    return nullptr;
}

/**
 * Write out all cached blocks for device dev.
 */
bool block_cache_t::flush(deviceno_t dev)
{
    cache_block_t* blk = blocks.lru;
    cache_block_t* prev_blk = 0;
    while (blk) {
        if (blk->is_busy() || (blk->device != dev)) {
            std::cerr << "NOT flushing block " << blk->block_num << " because busy? " << blk->is_busy() << ", or wrong dev? " << (blk->device != dev) << std::endl;
            blk = blk->next_mru;
            continue;
        }

        std::cerr << "Flushing block " << blk->block_num << " of device " << dev << std::endl;

        if (write_blocks(dev, blk->block_num, blk->data, 1, blk->block_size) != blk->block_size)
            return false;

        prev_blk = blk;
        blk = blk->next_mru;
        prev_blk->unlink_from(&blocks);
        cache[std::make_pair(dev, prev_blk->block_num)] = 0;
        delete prev_blk;
    }
    return true;
}

size_t block_cache_t::unwritten_blocks()
{
    size_t n = 0;
    cache_block_t* blk = blocks.lru;
    while (blk) {
        ++n;
        blk = blk->next_mru;
    }
    return n;
}

/**
 * Blocks we are trying to get may be either busy, locked or dirty. In either case, we cannot discard them and reuse for anything.
 * We also need to keep track that the blocks we've taken are of correct size, and if not - reallocate them.
 */
std::vector<cache_block_t*> block_cache_t::get_blocks(size_t nblocks, size_t block_size)
{
    if (nblocks > max_blocks)
        throw std::runtime_error("Cannot allocate more blocks than allowed in the cache in total!");

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
            blk->unlink_from(&blocks);
            ret.push_back(blk);
            --nblocks;
            break;
        }
    }

    return ret;
}

void block_cache_t::set_device_block_size(deviceno_t dev, size_t block_size)
{
    device_block_sizes[dev] = block_size;
}

size_t block_cache_t::get_block_size(deviceno_t dev)
{
    return device_block_sizes[dev];
}

size_t block_cache_t::byte_read(deviceno_t device, off_t byte_offset, char* data, size_t nbytes)
{
    return -1;
}

size_t block_cache_t::byte_write(deviceno_t device, off_t byte_offset, const char* data, size_t nbytes)
{
    // find the block size for device
    // then do a cached_write
    size_t block_size = get_block_size(device);
    // For now assume precise block-addressing; in reality for added flexibility byte_writes should allow uneven positions and sizes.
    assert(block_size);
    assert((byte_offset % block_size) == 0);
    assert((nbytes % block_size) == 0);
    return cached_write(device, byte_offset / block_size, data, nbytes / block_size, block_size);
}

/**
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
                    memutils::copy_memory(entry->data, buffer, block_size); // Update read data with cache data (e.g. dirty blocks).
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
            entry->unlink_from(&blocks); // Remove it from the list it is in, because it's going to be modified.
            memutils::copy_memory(buffer, entry->data, block_size); // FIXME: replace this with a visitor pattern?
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
                throw std::runtime_error("Read blocks from physical media failed! [make it nonfatal]");

            // create new blocks for just read data
            // add new block to the cache, evicting LRU entries as needed
            auto ents = get_blocks(block_stripe, block_size);

            if (ents.size() < block_stripe)
                throw std::runtime_error("Couldn't get enough block cache entries.");

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
    size_t written = 0;

    std::cerr << "cached_write(dev " << device << ", block " << block_n << ", nblocks " << nblocks << ", block_size " << block_size << ")" << std::endl;

    while (nblocks)
    {
        entry = block_lookup(device, block_n);
        if (entry)
        {
            assert(entry->block_size == block_size);
            std::cerr << "Block is found in the cache." << std::endl;
            entry->unlink_from(&blocks); // Remove it from the list it is in, because it's going to be modified.
            memutils::copy_memory(entry->data, buffer, block_size); // FIXME: replace this with a visitor pattern?

            entry->set_dirty();
            entry->device = device;
            entry->block_num = block_n;

            // Add block back at the start of the MRU list.
            entry->link_at_mru(&blocks);
            cache[std::make_pair(device, block_n)] = entry;
        }
        else
        {
            // Block is not found in the cache, create a new one (potentially pushing older blocks out of cache).
            auto ents = get_blocks(1, block_size);

            if (ents.size() < 1)
                throw std::runtime_error("get_blocks failed");

            std::cerr << "New blocks allocated: " << ents.size() << std::endl;

            entry = ents[0];
            memutils::copy_memory(entry->data, buffer, block_size);

            entry->set_dirty();
            entry->device = device;
            entry->block_num = block_n;

            // Add block back at the start of the MRU list.
            entry->link_at_mru(&blocks);
            cache[std::make_pair(device, block_n)] = entry;
        }

        block_n++;
        nblocks--;
        buffer += block_size;
        written += block_size;
    }

    return written;
}
