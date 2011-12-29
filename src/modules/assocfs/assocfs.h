//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2011, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
/*!
 * @brief Associative filesystem.
 *
 * A semantic net with objects and indexed metadata about objects.
 * DB-like structure for fast information retrieval and cross-referencing.
 * Multi-storage scalability, inter-device syncronization support through versioning/merging.
 *
 * Underlying structure similar to btrfs - btree with shadowing/cow.
 * File objects operations mostly similar to btrfs.
 */
#pragma once

namespace assocfs {

struct superblock_t
{
    uint8_t  magic[8]; //!< 'AssocFS.'
    uint64_t root_tree;
};

struct block_header_t
{
    uint8_t  csum[32]; //!< SHA256 block checksum.
    uint8_t  fsid[16]; //!< UUID of parent filesystem.
    uint64_t blocknr;
    uint64_t flags;
    char     chunk_tree_uid[16];
    uint64_t generation;
    uint64_t owner;
    uint32_t nritems;
    uint8_t  level;
};

// Key components are ordered from most significant (object_id) to least significant (offset).
struct key_t
{
    uint64_t object_id;
    uint8_t  type;
    uint64_t offset;
};

struct item_t
{
    key_t    key;
    uint32_t offset;
    uint32_t size;
};

struct file_extent_item_t : public item_t
{
    uint64_t generation;
    uint64_t start_block;
    uint64_t n_blocks;
};

// file data: extents or inline in btree.
// file metadata: extents or inline in btree.
// file cross-reference indices: extents, referencing extattrs keys
// extattrs metadata: separate btree
// 

/*
http://leaf.dragonflybsd.org/mailarchive/kernel/2007-10/msg00006.html

HAMMER objects revolve around the concept of an object identifier.
The obj_id is a 64 bit quantity which uniquely identifies a filesystem
object for the entire life of the filesystem.  This uniqueness allows
backups and mirrors to retain varying amounts of filesystem history by
removing any possibility of conflict through identifier reuse.  HAMMER
typically iterates object identifiers sequentially and expects to never
run out.  At a creation rate of 100,000 objects per second it would
take HAMMER around 6 million years to run out of identifier space.
The characteristics of the HAMMER obj_id also allow HAMMER to operate
in a multi-master clustered environment.
*/









}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
