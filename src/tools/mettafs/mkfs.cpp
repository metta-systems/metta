//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2011, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "block_device.h"
#include "block_device_mapper.h"
#include "block_cache.h"
#include <uuid/uuid.h>
#include "superblock.h"
#include "memutils.h"
#include "fourcc.h"
#include "macros.h"
#include <iostream>
#include <cassert>

// SHA256 from openssl
#include <openssl/sha.h>

//raiser/btrfs style blocks:

// use 4096 kb block size (or even 64kb?)
// fill it up from two sides
// left is keys, right is values

// keys are fixed-size, hash id plus a block offset and size of the value
//

// block_cache_t calls block device by a given id (via read_blocks and write_blocks)
// block_device_mapper_t transforms I/O requests from block_cache_t into calls on appropriate device
// block_device_t is uncached
// VFS layer can then read and write device blocks via the cache layer,
// VFS -> block_cache -> device_mapper -> block_device

class vfs_t
{
    block_device_mapper_t device_mapper; // decouple block cache from actual block devices

public:
    vfs_t() : device_mapper() {}

    void set_cache(block_cache_t& cache)
    {
        cache.set_device_mapper(device_mapper);
        device_mapper.set_cache(cache);
    }

    deviceno_t mount(block_device_t& dev, const char* name)
    {
        device_mapper.map_device(dev, name); // atomically assign an unique device id
        return mounted(name);
    }

    bool unmount(const char* name)
    {
        return device_mapper.unmap_device(mounted(name));
    }

    deviceno_t mounted(const char* name)
    {
        return device_mapper.resolve_device(name);
    }

    size_t read(deviceno_t device, off_t byte_offset, char* buffer, size_t size)
    {
        // LOCK
        block_cache_t& cache = device_mapper.get_cache();
        return cache.byte_read(device, byte_offset, buffer, size);
    }
    size_t write(deviceno_t device, off_t byte_offset, const char* buffer, size_t size)
    {
        // LOCK
        block_cache_t& cache = device_mapper.get_cache();
        return cache.byte_write(device, byte_offset, buffer, size);
    }
};

static vfs_t vfs;


static const int sectorsize = 4096;
static const int nodesize = 4096;
static const int leafsize = 4096;
static const int BLOCK_SIZE = 4096;

/*
 * fs super
 * +-root
 *   +-top of last objids tree
 * +-extents root
 *   +-top of extents tree? - ext tree address sorted with extent size, two adjacent keys define space used and free between them.
 *
 * objid key
 * +-objid+1 name item (??)
 * +-objid+2 tag list item
 * +-objid+3 file extents item
 * +-objid+4 etcetc
 *
 *
 * root: objid -> metadata lists mapping
 * extents: objid -> blocklist mapping
 *   extents are hashed, extents tree id is hash of hashes for faster calculation of updates. all extents are stored
 *   in a list together with the hashes/ids.
 */

void calc_checksum(btree_header_common_t& node, size_t bytes)
{
    SHA256_CTX ctx;
    memutils::fill_memory(node.checksum, 0, sizeof(node.checksum));
    SHA256_Init(&ctx);
    SHA256_Update(&ctx, (void *)&node, bytes);
    SHA256_Final(node.checksum, &ctx);
}

extern "C" void panic(const char* message, const char* file, uint32_t line)
{
    printf("%s\n", message);
    exit(-1);
}

int create_fs(deviceno_t device, size_t num_bytes, const char* label)
{
    fs_superblock_t super;
    memutils::fill_memory(&super, 0, sizeof(super));

    num_bytes = (num_bytes / sectorsize) * sectorsize;

    super.version = 1;
//     uint8_t   checksum[CHECKSUM_SIZE];  // [  4] block data checksum
    uuid_generate(super.fsid);
    super.block_offset = 0;             // which block this node is supposed to live in
    super.flags = 0;                    // [ 60] not related to validity, but matches generic header format for different trees.
    super.magic = Magic64BE<'M','e','T','T','a','F','S','1'>::value;
    super.generation = 1;
    super.root = 1 * sectorsize;              // [ 84] location of "root of roots" tree
    super.total_bytes = num_bytes;
    super.bytes_used = 0;
    super.sector_size = sectorsize;
    super.node_size = nodesize;
    super.leaf_size = leafsize;
    super.checksum_type = CHECKSUM_TYPE_SHA1;
    super.root_level = 1;
//     dev_item_t dev_item;        // [123]
    memutils::copy_string(super.label, label, sizeof(super.label));

    calc_checksum(super, sizeof(super));
    vfs.write(device, 0, reinterpret_cast<const char*>(&super), BLOCK_SIZE); //sizeof(super));

    // generate first root of roots tree
    fs_node_t root_node;
    memutils::fill_memory(&root_node, 0, sizeof(root_node));
    root_node.version = 1;
    memutils::copy_memory(root_node.fsid, super.fsid, sizeof(root_node.fsid));
    root_node.block_offset = 1*sectorsize;             // which block this node is supposed to live in
    root_node.level = 1;
    root_node.generation = 1;
    uuid_generate(root_node.chunk_tree_uuid);
    root_node.owner = 0;
    root_node.numItems = 9999;
    calc_checksum(root_node, sizeof(root_node));

    // stuff some nodes right next to the root node to make it load faster.

    vfs.write(device, BLOCK_SIZE, reinterpret_cast<const char*>(&root_node), BLOCK_SIZE); //sizeof(root_node));

    return 1;
}

int main(int argc, char** argv)
{
    if (argc != 4)
    {
        std::cerr << "mkfs deviceName <create 1 or 0> <byte size>" << std::endl;
        return 111;
    }

    const char* fname = argv[1];
    int create = atoi(argv[2]);
    size_t size = atoi(argv[3]);
    block_cache_t cache(256);
    block_device_t dev(fname, create, BLOCK_SIZE);

    vfs.set_cache(cache);
    deviceno_t device = vfs.mount(dev, "arbitrary_name");

    std::cerr << "Unwritten blocks before: " << cache.unwritten_blocks() << std::endl;

    create_fs(device, size, "test_fs");

    std::cerr << "Unwritten blocks after: " << cache.unwritten_blocks() << std::endl;

    vfs.unmount("arbitrary_name");
    std::cerr << "Unwritten blocks after unmount: " << cache.unwritten_blocks() << std::endl;
    return 0;
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
