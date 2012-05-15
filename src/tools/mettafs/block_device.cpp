//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2012, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "block_device.h"
#include "macros.h"
#include <fstream>
#include <iostream>
#include <cassert>

using namespace std;

block_device_t::block_device_t(const std::string& name, bool create, blocksize_t bs, blockno_t numBlocks)
    : storageFileName(name)
    , blockSize(bs)
    , numBlocks(numBlocks)
{
    storageFile = new fstream(storageFileName.c_str(), create ? ios::out|ios::trunc|ios::binary : ios::out|/*ios::nocreate|*/ios::binary);
}

block_device_t::~block_device_t()
{
    if (storageFile)
        close();
}

void block_device_t::close()
{
    assert(storageFile);
    assert(storageFile->good());
    storageFile->close();
    delete storageFile; // TODO: no matching open()
    storageFile = 0;
}

/**
 * Return time in nanoseconds it would take to seek from current block position to seekTo.
 */
int block_device_t::seek_time(blockno_t seekTo)
{
    UNUSED(seekTo);
    return 0;
}

// hey, seek doesn't really seek, it just emulates the delay it would take the device to seek from current position to block
// with presence of seek_time it's probably not needed.
void block_device_t::seek(blockno_t block)
{
    UNUSED(block);
}

block_device_t::blockno_t block_device_t::pos()
{
    return 0;
}

/**
 * Read and write functions operate on whole blocks of specific size.
 */
block_device_t::blocksize_t block_device_t::read_block(block_device_t::blockno_t block, char* buffer, block_device_t::blocksize_t bytes)
{
    if (bytes % blockSize)
    {
        std::cerr << "block read of non-block size buffer" << std::endl;
        return 0;
    }
    storageFile->seekg(block * blockSize);
    storageFile->read(buffer, bytes);
    return bytes;
}

void block_device_t::write_block(block_device_t::blockno_t block, const char* buffer, block_device_t::blocksize_t bytes)
{
    if (bytes % blockSize)
    {
        std::cerr << "block write of non-block size buffer" << std::endl;
        return;
    }
    storageFile->seekp(block * blockSize);
    storageFile->write(buffer, bytes);
}
