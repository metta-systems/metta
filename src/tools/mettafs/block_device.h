//
// Part of Metta OS. Check http://atta-metta.net for latest version.
//
// Copyright 2007 - 2012, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include <string>
#include <fstream>

/**
 * The block device emulates a disk block device with configured block size and access times. It uses a regular file in
 * a filesystem to store data.
 */
class block_device_t
{
public:
    /**
     * blockno_t type designates a block address type, main unit of addressing in a block device.
     */
    typedef unsigned long long blockno_t;
    typedef size_t blocksize_t;

    block_device_t(const std::string& storageFile, bool create = false, blocksize_t blockSize = 0, blockno_t numBlocks = 0);
    virtual ~block_device_t();

    void close();

    /**
     * Return time in nanoseconds it would take to seek from current block position to seekTo.
     */
    int seek_time(blockno_t seekTo);

    void seek(blockno_t block);
    blockno_t pos();

    blocksize_t block_size() { return blockSize; }

    /**
     * Read and write functions operate on whole blocks of specific size.
     */
    blocksize_t read_block(blockno_t block, char* buffer, blocksize_t bufSize);
    void write_block(blockno_t block, const char* buffer, blocksize_t bytes);

private:
    std::string storageFileName;
    std::fstream* storageFile;
    blocksize_t blockSize;
    blockno_t numBlocks;
};

