//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2011, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "block_device.h"
#include "macros.h"
#include <fstream>

using namespace std;

block_device_t::block_device_t(const std::string& name, bool create, blocksize_t blockSize, blockno_t numBlocks)
    : storageFileName(name)
    , blockSize(blockSize)
    , numBlocks(numBlocks)
{
    storageFile = new fstream(storageFileName, create ? ios::out|ios::trunc|ios::binary : ios::out|/*ios::nocreate|*/ios::binary);
}

block_device_t::~block_device_t()
{
    delete storageFile;
}

/*!
 * Return time in nanoseconds it would take to seek from current block position to seekTo.
 */
int block_device_t::seek_time(blockno_t seekTo)
{
    UNUSED(seekTo);
    return 0;
}

void block_device_t::seek(blockno_t block)
{
    UNUSED(block);
}

block_device_t::blockno_t block_device_t::pos()
{
    return 0;
}

/*!
 * Read and write functions operate on whole blocks of specific size.
 */
block_device_t::blocksize_t block_device_t::read_block(block_device_t::blockno_t block, char* buffer)
{
    UNUSED(block);
    UNUSED(buffer);
    return 0;
}

void block_device_t::write_block(block_device_t::blockno_t block, const char* buffer, block_device_t::blocksize_t bytes)
{
    UNUSED(block);
    UNUSED(buffer);
    UNUSED(bytes);
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
