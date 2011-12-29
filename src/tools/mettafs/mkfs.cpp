//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2011, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "block_device.h"
#include "block_cache.h"
#include <uuid/uuid.h>
#include "superblock.h"
#include "memutils.h"
#include "fourcc.h"
#include "macros.h"
#include <stdio.h>

// block_cache_t calls block device by a given id (via read_blocks and write_blocks)
// block_device_mapper_t transforms I/O requests from block_cache_t into calls on appropriate device
// block_device_t is uncached
// VFS layer can then read and write device blocks via the cache layer, 
// VFS -> block_cache -> device_mapper -> block_device

class block_device_mapper_t
{
    block_cache_t* cache;

public:
    block_device_mapper_t() {}
    void map_device(block_device_t&, const char*) {}
    deviceno_t resolve_device(const char*) const { return -1; }
    void set_cache(block_cache_t& c) { cache = &c; }
    block_cache_t& get_cache() const { return *cache; }
};

class vfs_t
{
    block_device_mapper_t device_mapper; // decouple block cache from actual block devices

public:
    vfs_t() : device_mapper() {}

    void set_cache(block_cache_t& cache) { device_mapper.set_cache(cache); }
    deviceno_t mount(block_device_t& dev, const char* name)
    {
        device_mapper.map_device(dev, name); // atomically assign an unique device id
        return mounted(name);
    }

    bool unmount(const char* name)
    {
        // return device_mapper.flush_cache(mounted(name));
        return true;
    }

    deviceno_t mounted(const char* name)
    {
        return device_mapper.resolve_device(name);        
    }

    size_t read(deviceno_t device, off_t byte_offset, void* buffer, size_t size)
    {
        // LOCK
        block_cache_t& cache = device_mapper.get_cache();
        return cache.byte_read(device, byte_offset, buffer, size);
    }
    size_t write(deviceno_t device, off_t byte_offset, void* buffer, size_t size)
    {
        // LOCK
        block_cache_t& cache = device_mapper.get_cache();
        return cache.byte_write(device, byte_offset, buffer, size);
    }
};

static vfs_t vfs;


const int sectorsize = 4096;
const int nodesize = 4096;
const int leafsize = 4096;

void calc_checksum(fs_superblock_t& super)
{
    // TODO: calc proper SHA1 checksum over sb
    super.checksum_type = CHECKSUM_TYPE_SHA1;
}

extern "C" void panic(const char* message, const char* file, uint32_t line)
{
    printf("%s\n", message);
    exit(-1);
}

int make_fs(deviceno_t device, size_t num_bytes, const char* label)
{
    fs_superblock_t super;

    num_bytes = (num_bytes / sectorsize) * sectorsize;

    memutils::fill_memory(&super, 0, sizeof(super));

    super.version = 1;
//     uint8_t   checksum[CHECKSUM_SIZE];  // [  4] block data checksum
    uuid_generate(super.fsid);
    super.block_offset = 0;             // which block this node is supposed to live in
    super.flags = 0;                    // [ 60] not related to validity, but matches generic header format for different trees.
    super.magic = Magic64BE<'M','e','T','T','a','F','S','1'>::value;
    super.generation = 1;
//     uint64_t root;              // [ 84] location of "root of roots" tree
    super.total_bytes = num_bytes;
    super.bytes_used = 0;
    super.sector_size = sectorsize;
    super.node_size = nodesize;
    super.leaf_size = leafsize;
    super.checksum_type = CHECKSUM_TYPE_SHA1;
    super.root_level = 1;
    calc_checksum(super);
//     dev_item_t dev_item;        // [123]
    memutils::copy_string(super.label, label, sizeof(super.label));

    vfs.write(device, 0, &super, sizeof(super));

    return 1;
}

int main(int argc, char** argv)
{
    // mkfs deviceName <create 1 or 0> <byte size>
    if (argc != 4)
        return 111;

    const char* fname = argv[1];
    int create = atoi(argv[2]);
    size_t size = atoi(argv[3]);
    block_cache_t cache(256);
    block_device_t dev(fname, create);

    vfs.set_cache(cache);
    deviceno_t device = vfs.mount(dev, "arbitrary_name");
 
    //vfs.write(device, buffer, offset, size);

    make_fs(device, size, "test_fs");

    vfs.unmount("arbitrary_name");
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
