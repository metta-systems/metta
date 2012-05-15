//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2012, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include <map>
#include <cassert>
#include "block_cache.h"

/**
 * Device mapper can convert between abstract device numbers and actual devices performing I/O.
 */
class block_device_mapper_t
{
    block_cache_t* cache;
    static int next_device;
    std::map<const char*, deviceno_t> device_ids;
    std::map<deviceno_t, block_device_t*> devices;

public:
    block_device_mapper_t() {}
    void map_device(block_device_t& dev, const char* name)
    {
        deviceno_t d = ++next_device;
        device_ids[name] = d;
        devices[d] = &dev;
        cache->set_device_block_size(d, dev.block_size());
    }
    bool unmap_device(deviceno_t dev)
    {
        block_device_t* device = devices[dev];
        assert(device);
        cache->flush(dev);
        device->close();
        return true;
    }
    deviceno_t resolve_device(const char* name) /*const*/ { return device_ids[name]; }
    void set_cache(block_cache_t& c) { cache = &c; }
    block_cache_t& get_cache() const { return *cache; }

    /**
     * Perform block read on actual device.
     */
    size_t read(deviceno_t dev, off_t block_no, char* buffer, size_t size)
    {
        // LOCK
        block_device_t* device = devices[dev];
        assert(device);
        return device->read_block(block_no, buffer, size);
    }
    /**
     * Perform block write on actual device.
     */
    size_t write(deviceno_t dev, off_t block_no, const char* buffer, size_t size)
    {
        // LOCK
        block_device_t* device = devices[dev];
        assert(device);
        /*return*/ device->write_block(block_no, buffer, size);
        return size;
    }
};
